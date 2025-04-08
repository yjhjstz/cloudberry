/*-------------------------------------------------------------------------
 *
 * inmem_smgr.h
 *
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *-------------------------------------------------------------------------
 */
#ifndef INMEM_SMGR_H
#define INMEM_SMGR_H

extern bool am_wal_redo_postgres;

extern smgr_hook_type inmen_prev_smgr_hook;
extern const struct f_smgr inmem_smgr;
extern void smgr_init_inmem(void);
extern void inmem_smgr_init(SMgrRelation reln, BackendId backend, SMgrImpl which, Relation rel);

#endif /* INMEM_SMGR_H */
