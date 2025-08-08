#include "gopherresowner.h"
#include "access/xact.h"
#include "cdb/cdbvars.h"
#include "c.h"
#include "util.h"
#include <gopher/gopher.h>

static gopher_context_handle_t *
gopher_create_context_handle(bool gp_is_writer);

static void
gopher_destroy_context_handle(gopher_context_handle_t *h);

static void
gopher_cleanup_context_handle(gopher_context_handle_t *h);

static void
gopher_context_abort_callback(ResourceReleasePhase phase,
					   bool isCommit,
					   bool isTopLevel,
					   void *arg);

static void
gopher_clear_list_result(int gp_session_id, int cid, bool gp_is_writer);

static gopher_context_handle_t *open_gopher_context_handles;

static bool gopher_context_resowner_callback_registered;

static gopher_context_handle_t *
gopher_create_context_handle(bool gp_is_writer)
{
	gopher_context_handle_t *h;
	h = MemoryContextAlloc(TopMemoryContext, sizeof(gopher_context_handle_t));
	h->cid = gp_command_count;
	h->gp_is_writer = gp_is_writer;
	h->owner = CurrentResourceOwner;
	h->next = open_gopher_context_handles;
	h->prev = NULL;
	if (open_gopher_context_handles)
		open_gopher_context_handles->prev = h;
	open_gopher_context_handles = h;

	return h;
}

/*
 * Close any open handles on abort.
 */
static void
gopher_destroy_context_handle(gopher_context_handle_t *h)
{
	gopher_cleanup_context_handle(h);
}

/*
 * Cleanup open handles.
 */
static void
gopher_cleanup_context_handle(gopher_context_handle_t *h)
{
	/* unlink from linked list first */
	if (h == NULL)
	{
		return;
	}
	if (h->prev)
		h->prev->next = h->next;
	else
		open_gopher_context_handles = h->next;
	if (h->next)
		h->next->prev = h->prev;

	gopher_clear_list_result(gp_session_id, h->cid, h->gp_is_writer);

	pfree(h);
}

/*
 * Close any open handles on abort.
 */
static void
gopher_context_abort_callback(ResourceReleasePhase phase,
					   bool isCommit,
					   bool isTopLevel,
					   void *arg)
{
	gopher_context_handle_t *curr;
	gopher_context_handle_t *next;

	if (phase != RESOURCE_RELEASE_AFTER_LOCKS)
		return;

	next = open_gopher_context_handles;
	while (next)
	{
		curr = next;
		next = curr->next;

		if (curr->owner == CurrentResourceOwner)
		{
			if (isCommit)
				elog(WARNING, "datalake execute-type external table reference leak: %p still referenced", curr);
			gopher_cleanup_context_handle(curr);
		}
	}
}

gopher_context_handle_t* gopher_registe_resource_context(bool gp_is_writer)
{
	if (!gopher_context_resowner_callback_registered)
	{
		RegisterResourceReleaseCallback(gopher_context_abort_callback, NULL);
		gopher_context_resowner_callback_registered = true;
	}
	return gopher_create_context_handle(gp_is_writer);
}

void cleanup_gopher_resource_context(gopher_context_handle_t* h)
{
	gopher_destroy_context_handle(h);
}

void gopher_clear_list_result(int gp_session_id, int cid, bool gp_is_writer)
{
	char hostAddress[MAXPGPATH + 1];
	DatalakeGetGopherSocketPath(hostAddress);
	gopherAdmin admin = gopherCreateAdmin(hostAddress);
    if (admin == NULL)
    {
        elog(LOG, "External table clear list result: failed to create admin handle for '%s' (cid=%d, session=%u) %s",
                 hostAddress, cid, gp_session_id, gopherGetLastError());
        return;
    }
    if (gopherAdminClearListResult(admin, gp_session_id, cid, gp_is_writer) == -1)
    {
        elog(LOG, "External table clear list result failed (cid=%d, session=%u) %s",
			cid, gp_session_id, gopherGetLastError());
    }
	gopherDeleteAdmin(admin);
}
