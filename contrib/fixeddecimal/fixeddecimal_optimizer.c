#include "postgres.h"

#include "catalog/pg_namespace.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_type.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "parser/parse_agg.h"
#include "utils/builtins.h"
#include "utils/catcache.h"
#include "utils/guc.h"
#include "utils/syscache.h"

#define FIXEDDECIMAL_SCALE 2

static Oid fixeddecimal_type_oid = InvalidOid;
static Oid numeric_fixeddecimal_func_oid = InvalidOid;
static bool enable_fixeddecimal_optimize = false;
struct FuncOidMap {
	Oid numeric_fn_oid;
	Oid fixeddecimal_fn_oid;
};
static struct FuncOidMap func_oid_mappings[] = {
	{InvalidOid, InvalidOid}, /* sum(numeric) -> sum(fixeddecimal) */
	{InvalidOid, InvalidOid}, /* avg(numeric) -> avg(fixeddecimal) */
};

#define SUM__NUMERIC_OID func_oid_mappings[0].numeric_fn_oid
#define AVG__NUMERIC_OID func_oid_mappings[1].numeric_fn_oid
#define SUM__FIXEDDECIMAL_OID func_oid_mappings[0].fixeddecimal_fn_oid
#define AVG__FIXEDDECIMAL_OID func_oid_mappings[1].fixeddecimal_fn_oid

extern Datum numeric_fixeddecimal(PG_FUNCTION_ARGS);
extern void _PG_init(void);

static inline Oid
map_func_oid(Oid numeric_fn_oid)
{
	for (int i = 0, n = lengthof(func_oid_mappings); i < n; i++) {
		if (func_oid_mappings[i].numeric_fn_oid == numeric_fn_oid)
			return func_oid_mappings[i].fixeddecimal_fn_oid;
	}
	return InvalidOid;
}

static inline bool
is_numeric_optimizable(Oid typid, int32 typmod)
{
	if (typid != NUMERICOID || typmod < 0)
		return false;

	/* Check if scale is within fixeddecimal limits */
	int precision = (typmod >> 16) & 0xFFFF;
	int scale = typmod & 0xFFFF;

	return scale == FIXEDDECIMAL_SCALE + 4 && precision < 19;
}

static inline bool
is_integer_type(Oid typid)
{
	/* fixeddecimal doesn't define any operator with INT8, ignore it. */
	return typid == INT2OID || typid == INT4OID;
}

static Oid
find_proc_by_name(const char *funcname, oidvector *argtypes, Oid namespace_oid)
{
	HeapTuple tup;
	Form_pg_proc form;
	Oid fn_oid;

	tup = SearchSysCache3(PROCNAMEARGSNSP,
						  CStringGetDatum(funcname),
						  PointerGetDatum(argtypes),
						  ObjectIdGetDatum(namespace_oid));
	if (!HeapTupleIsValid(tup))
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_FUNCTION),
				 errmsg("function %s not found", funcname)));

	form = (Form_pg_proc) GETSTRUCT(tup);
	fn_oid = form->oid;

	Assert(form->pronargs == 1);

	ReleaseSysCache(tup);

	return fn_oid;
}

static void
initialize_fixeddecimal_oids_(void)
{
	Oid oids[1];
	Oid ns_oid = PG_CATALOG_NAMESPACE;
	oidvector *argtypes = NULL;

	if (!OidIsValid(fixeddecimal_type_oid))
	{
		/* Lookup fixeddecimal type Oid */
		HeapTuple tup;
		Form_pg_type form;

		tup = SearchSysCache2(TYPENAMENSP,
							  CStringGetDatum("fixeddecimal"),
							  ObjectIdGetDatum(ns_oid));
		if (!HeapTupleIsValid(tup))
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_OBJECT),
					 errmsg("fixeddecimal type not found")));

		form = (Form_pg_type) GETSTRUCT(tup);
		fixeddecimal_type_oid = form->oid;

#ifdef USE_ASSERT_CHECKING
		Assert(form->typlen == 8); /* fixeddecimal should be 8 bytes */
		Assert(form->typbyval);    /* fixeddecimal should be passed by value */
#endif

		ReleaseSysCache(tup);
	}

	{
		oids[0] = fixeddecimal_type_oid;
		argtypes = (oidvector *) buildoidvector(oids, 1);

		/* Lookup sum(fixeddecimal)/avg(fixeddecimal) function Oid */
		SUM__FIXEDDECIMAL_OID = find_proc_by_name("sum", argtypes, ns_oid);
		AVG__FIXEDDECIMAL_OID = find_proc_by_name("avg", argtypes, ns_oid);

		pfree(argtypes);
	}

	{
		/* Lookup sum(numeric)/avg(numeric) function Oid */
		oids[0] = NUMERICOID;
		argtypes = (oidvector *) buildoidvector(oids, 1);

		SUM__NUMERIC_OID = find_proc_by_name("sum", argtypes, PG_CATALOG_NAMESPACE);
		AVG__NUMERIC_OID = find_proc_by_name("avg", argtypes, PG_CATALOG_NAMESPACE);
		numeric_fixeddecimal_func_oid = find_proc_by_name("numeric_fixeddecimal", argtypes, ns_oid);

		pfree(argtypes);
	}
}

static inline void
initialize_fixeddecimal_oids(void)
{
	if (!OidIsValid(fixeddecimal_type_oid))
		initialize_fixeddecimal_oids_();
}

static inline Oid
get_fixeddecimal_type_oid(void)
{
	initialize_fixeddecimal_oids();
	return fixeddecimal_type_oid;
}

static inline Oid
get_numeric_fixeddecimal_func_oid(void)
{
	initialize_fixeddecimal_oids();
	return numeric_fixeddecimal_func_oid;
}

struct check_support_context {
	ParseState *pstate;
	char result; // 'i', 'n', 'x'
};

static void
check_support_context_update(struct check_support_context *context, char result) {
	Assert(result == 'i' || result == 'n' || result == 'x');
	switch (context->result)
	{
	case 0: /* initial state */
	case 'i':
		context->result = result;
		break;
	case 'n':
		if (result == 'x')	
			context->result = result;
		break;
	default:
		break;
	}
}

static inline char
type_to_result(Oid typid, int32 typmod)
{
	if (is_integer_type(typid)) return 'i';
	if (is_numeric_optimizable(typid, typmod)) return 'n';
	return 'x';
}

static char *
get_oprname(Oid oprid, char oprname[]) {
	HeapTuple tup;
	Form_pg_operator form;

	tup = SearchSysCache1(OPEROID, ObjectIdGetDatum(oprid));
	if (!HeapTupleIsValid(tup))
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_FUNCTION),
				 errmsg("operator with OID %u not found", oprid)));

	form = (Form_pg_operator) GETSTRUCT(tup);
	strncpy(oprname, NameStr(form->oprname), NAMEDATALEN);
	ReleaseSysCache(tup);
	return oprname;
}

static bool
check_support_simple(Node *node, struct check_support_context *context)
{
	Oid typid = exprType(node);
	int32 typmod = exprTypmod(node);

	char result = type_to_result(typid, typmod);
	check_support_context_update(context, result);
	return result == 'x';	
}

static bool
check_support_funcexpr(FuncExpr *func, struct check_support_context *context)
{
	if (is_integer_type(func->funcresulttype))
	{
		check_support_context_update(context, 'i');
		return false;
	}
	if (func->funcresulttype != NUMERICOID)
		goto unsupport;

	if (func->funcformat == COERCE_IMPLICIT_CAST ||
		func->funcformat == COERCE_EXPLICIT_CAST) {

		Node *castsource = (Node *) linitial(func->args);
		if (IsA(castsource, Const)) {
			// cast const to source
			Const *c = (Const *) castsource;
			Oid typid = c->consttype;

			// cast integer to numeric
			// cast func will be discarded.
			if (is_integer_type(typid)) {
				check_support_context_update(context, 'i');
				return false;
			}

			// cast numeric to numeric, target scale must be 2
			if (typid != NUMERICOID || list_length(func->args) < 2) goto unsupport;

			Const *c_typmod = (Const *) list_nth(func->args, 1);
			if (!IsA(c_typmod, Const) ||
				 c_typmod->constisnull ||
				 c_typmod->consttype != INT4OID)
				goto unsupport;

			if (!is_numeric_optimizable(NUMERICOID, DatumGetInt32(c_typmod->constvalue)))
				goto unsupport;
			
			check_support_context_update(context, 'n');
			return false; // stop if it breaks the rule
		} else {
			return check_support_simple(castsource, context);
		}
		goto unsupport;
	} else {
		return check_support_simple((Node *) func, context);
	}

unsupport:
	context->result = 'x';
	return true;
}
// returns:
// 'i': integer
// 'n': numeric(P, 2)
// 'x': others, unsupport
static bool
check_support_fixeddecimal_walker(Node *node, struct check_support_context *context)
{
	ParseState *pstate = context->pstate;

	if (!node) return false;

	switch (nodeTag(node))
	{
	case T_FuncExpr:
		return check_support_funcexpr((FuncExpr *)node, context);

	case T_Const: /* fallthrough */
	case T_Var:
		return check_support_simple(node, context);

	case T_OpExpr: {
		OpExpr *op = (OpExpr *) node;
		Oid typid = op->opresulttype;
		ListCell *lc;

		int idx;
		int nargs = list_length(op->args);
		char types[2];

		Assert(nargs <= 2);
		if (!is_integer_type(typid) && typid != NUMERICOID)
			goto unsupport;

		foreach_with_count(lc, op->args, idx) {
			Node *arg = (Node *) lfirst(lc);
			struct check_support_context arg_context;

			arg_context.pstate = pstate;
			arg_context.result = 0;
			check_support_fixeddecimal_walker(arg, &arg_context);
			types[idx] = arg_context.result;

            // only support type of integers and numeric(P, 2)
			if (types[idx] != 'i' && types[idx] != 'n') goto unsupport;
		}

		switch (nargs) {
			default:
				Assert(false);
				goto unsupport;
			case 1:
				check_support_context_update(context, typid == NUMERICOID ? 'n' : 'i');
				return false;
			case 2: {
				char oprname[NAMEDATALEN];
				bool ok = false;

				Assert(types[0] == 'i' || types[0] == 'n');
				Assert(types[1] == 'i' || types[1] == 'n');
				Assert(is_integer_type(typid) || typid == NUMERICOID);

				get_oprname(op->opno, oprname);
				// only support +, -, * operators
				if (strcmp(oprname, "+") == 0 || strcmp(oprname, "-") == 0) {
                    // + or - between numerics and or integer are totally ok
					ok = true;
				} else if (strcmp(oprname, "*") == 0) {
					// multiply between numerics increases scale, unsupport
					ok = !(types[0] == 'n' && types[1] == 'n');
				}
				if (!ok) goto unsupport;

				check_support_context_update(context, typid == NUMERICOID ? 'n' : 'i');
				return false;
			}
		}
		return false;
	}
	default:
		goto unsupport;
	}

	return expression_tree_walker(node, check_support_fixeddecimal_walker, (void *) pstate);

unsupport:
	context->result = 'x';
	return true;
}

static bool
supports_fixeddecimal_optimization(ParseState *pstate, Aggref *agg, List *args)
{
	struct check_support_context context;
	bool support_agg;

	initialize_fixeddecimal_oids();
	support_agg = OidIsValid(map_func_oid(agg->aggfnoid)) &&
				  agg->aggtype == NUMERICOID && list_length(args) == 1;

	if (!support_agg) return false;

	context.pstate = pstate;
	context.result = 0;
	check_support_fixeddecimal_walker((Node *) linitial(args), &context);
	return context.result == 'n';
}

static Node *
make_conversion_node(Node *node)
{
	FuncExpr *fexpr;

	Assert(!IsA(node, List));
	Assert(is_numeric_optimizable(exprType(node), exprTypmod(node)));

	fexpr = makeNode(FuncExpr);

	fexpr->funcid = get_numeric_fixeddecimal_func_oid();
	fexpr->funcresulttype = get_fixeddecimal_type_oid();
	fexpr->funcretset = false;
	fexpr->funcvariadic = false;
	fexpr->funcformat = COERCE_EXPLICIT_CALL;
	fexpr->funccollid = InvalidOid;
	fexpr->inputcollid = InvalidOid;
	fexpr->args = list_make1(node);
	fexpr->location = exprLocation((Node *) node);
	fexpr->is_tablefunc = false;
	return (Node *) fexpr;
}

static Node *
replace_expr_to_fixeddecimal(Node *node, ParseState *pstate)
{
	Assert(node);
	switch (nodeTag(node)) {
	case T_Var:
	{
		// try to replace Var with type numeric(P, 2) to numeric_fixeddecimal(Var)
		Var *var = (Var *) node;
		Oid typid = var->vartype;

		if (typid != NUMERICOID) return node;

#ifdef USE_ASSERT_CHECKING
		int32 typmod = var->vartypmod;
		Assert((typmod & 0xFFFF) == 6);
		Assert(((typmod >> 16) & 0xFFFF) < 19);
#endif

		return make_conversion_node(node);
	}
	case T_FuncExpr:
	{
		// if function implicitly cast a constant integer, returns it without cast function
		FuncExpr *f = (FuncExpr *) node;

		if (f->funcresulttype != NUMERICOID) return node;

		if (f->funcformat == COERCE_IMPLICIT_CAST ||
			f->funcformat == COERCE_EXPLICIT_CAST)
		{
			Node *castsource = (Node *) linitial(f->args);
			if (IsA(castsource, Const))
			{
				Const *c = (Const *) castsource;
				if (c->consttype == NUMERICOID)
				{
					Const *cc;
					Datum casttarget;

					Assert(!c->constisnull);
					Assert(list_length(f->args) > 1);

					cc = (Const *) list_nth(f->args, 1);
					if (!IsA(cc, Const) || cc->consttype != INT4OID || cc->constisnull ||
						!is_numeric_optimizable(NUMERICOID, DatumGetInt32(cc->constvalue)))
						ereport(ERROR,
								(errmsg("fixeddecimal: inconsistent state by cast check")));
				
					// convert numeric to numeric(P, 2)
					casttarget = DirectFunctionCall2(numeric, c->constvalue, cc->constvalue);
					// convert numeric(P, 2) to fixeddecimal
					casttarget = DirectFunctionCall1(numeric_fixeddecimal, casttarget);
					return (Node *) makeConst(get_fixeddecimal_type_oid(),
											  -1,
											  InvalidOid,
											  8,
											  casttarget,
											  false,
											  true);
				}
				else
				{
					Assert(is_integer_type(c->consttype));
					return castsource;
				}
			} else {
				Oid typid = exprType(castsource);
				
				Assert(is_integer_type(typid) || is_numeric_optimizable(typid, exprTypmod(castsource)));

				return typid == NUMERICOID ?  make_conversion_node(castsource)
											: castsource;
			}
		} else {
			Oid typid = f->funcresulttype;
			
			Assert(is_integer_type(typid) || is_numeric_optimizable(typid, exprTypmod(node)));

			return typid == NUMERICOID ?  make_conversion_node(node) : node;
		}
		break;
	}
	case T_OpExpr:
	{
		OpExpr *op = (OpExpr *) node;
		ListCell *lc;
		List *new_args = NIL;
		Oid opr_typids[2] = {InvalidOid, InvalidOid};
		int idx;
		bool update_opr = false; // whether type of left/right operand is changed

		foreach_with_count(lc, op->args, idx) {
			Oid old_typid;
			Oid new_typid;
			Node *old_op;
			Node *new_op;

			old_op = (Node *) lfirst(lc);
			old_typid = exprType(old_op);

			new_op = replace_expr_to_fixeddecimal(old_op, pstate);
			new_typid = exprType(new_op);

			opr_typids[idx] = new_typid;
			if (!update_opr && old_typid != new_typid)
				update_opr = true;

			new_args = lappend(new_args, new_op);
		}
		op->args = new_args;
		if (!update_opr) return node;

		{
			// The type of left/right operand is changed,
			// so the OpExpr needs also update.

			OpExpr *new_op = makeNode(OpExpr);
			char oprname[NAMEDATALEN];
			struct catclist *catlist;
			Oid oprleft;
			Oid oprright;
			get_oprname(op->opno, oprname);
			
			Assert(list_length(new_args) <= 2);
			if (list_length(new_args) == 1) {
				oprleft = InvalidOid;
				oprright = opr_typids[0];
			} else {
				oprleft = opr_typids[0];
				oprright = opr_typids[1];

			}
			catlist = SearchSysCacheList3(OPERNAMENSP, CStringGetDatum(oprname),
										  ObjectIdGetDatum(oprleft),
										  ObjectIdGetDatum(oprright));
			if (catlist->n_members != 1) {
				ReleaseSysCacheList(catlist);
				ereport(ERROR,
						(errcode(ERRCODE_UNDEFINED_FUNCTION),
						 errmsg("operator %s(%s, %s) expect 1, found %d",
								oprname,
								oprleft == InvalidOid ? "-" : format_type_be(oprleft),
								oprright == InvalidOid ? "-" : format_type_be(oprright),
								catlist->n_members)));
			}

			// find new operator
			Form_pg_operator form = (Form_pg_operator) GETSTRUCT(&catlist->members[0]->tuple);
			new_op->opno = form->oid;
			new_op->opresulttype = form->oprresult;
			new_op->opretset = false;
			new_op->inputcollid = op->inputcollid;
			new_op->opcollid = op->opcollid;
			new_op->args = new_args;

			ReleaseSysCacheList(catlist);

			return (Node *) new_op;
		}
	}
	default:
		break;
	}

	return expression_tree_mutator(node, replace_expr_to_fixeddecimal, (void *) pstate);
}

static void
fixeddecimal_transform_agg_hook(ParseState *pstate, Aggref *agg, List **args)
{
	Node *new_args;
	Oid new_fnoid;
	bool replace_numeric;

	if (!enable_fixeddecimal_optimize) return;

	replace_numeric = supports_fixeddecimal_optimization(pstate, agg, *args);
	ereport(DEBUG1,
			(errmsg("fixeddecimal: replace_numeric %s",
					 replace_numeric ? "SUCCESS" : "FAIL")));

	if (!replace_numeric) return;

	new_args = replace_expr_to_fixeddecimal((Node *)*args, pstate);
	*args = castNode(List, new_args);

	Assert(agg->aggtype == NUMERICOID);
	new_fnoid = map_func_oid(agg->aggfnoid);
	if (!OidIsValid(new_fnoid))
		ereport(ERROR,
				(errcode(ERRCODE_CASE_NOT_FOUND),
				 errmsg("unexpected aggfnoid(%u)", agg->aggfnoid)));

	agg->aggfnoid = new_fnoid;
}

void _PG_init(void)
{
	/* define GUC: fixeddecimal.enable_optimizer */
	DefineCustomBoolVariable("fixeddecimal.enable_optimizer",
							 "Enable fixeddecimal aggregate optimization",
							 "When enabled, rewrite eligible sum/avg on numeric(P,2) to fixeddecimal variants",
							 &enable_fixeddecimal_optimize,
							 false, /* default */
							 PGC_USERSET, 0,
							 NULL, NULL, NULL);

	/* install hook (chain previous if any) */
	transform_aggregate_call_hook = fixeddecimal_transform_agg_hook;
}
