 #ifndef GOPHER_RESOWNER_H
 #define GOPHER_RESOWNER_H
 
 
 #include "postgres.h"
 #include "utils/resowner.h"
 
 typedef struct gopher_context_handle_t
 {
     int cid;
     bool gp_is_writer;
     ResourceOwner owner;	/* owner of this handle */
     struct gopher_context_handle_t *next;
     struct gopher_context_handle_t *prev;
 } gopher_context_handle_t;
 
 gopher_context_handle_t* gopher_registe_resource_context(bool gp_is_writer);
 
 void cleanup_gopher_resource_context(gopher_context_handle_t* h);
 
 #endif /* GOPHER_RESOWNER_H */