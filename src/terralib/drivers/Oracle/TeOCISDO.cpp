/*	Delegate OCI calls -- Ran Wei, Oracle SDO, 9/10/1998
	Copyright 1998, Oracle Corp. */



#include <oci.h>
#include <TeOCISDO.h>

//extern "C"
//{
//	#include <oci.h>
//	#include "sdo_oci.h"
//}





/********************************************************************************/

/* Part I -- OCI fundamental interfaces                                         */

/********************************************************************************/

/*
Note: OCIEnvCreate() should be used instead of the
OCIInitialize() and OCIEnvInit() calls.
OCIInitialize() and OCIEnvInit() calls will be supported
for backward compatibility.
*/

sword SDO_OCIEnvCreate(OCIEnv **envhpp, ub4 mode, dvoid *ctxp, 

						 dvoid *(*malocfp)(dvoid *ctxp, size_t size),

						dvoid *(*ralocfp)(dvoid *ctxp, dvoid *memptr, size_t newsize),

						void (*mfreefp)(dvoid *ctxp, dvoid *memptr),
						
						size_t xtramensz, dvoid **usrmempp)

{

	return OCIEnvCreate (envhpp, mode, ctxp, malocfp, ralocfp,  mfreefp, xtramensz, usrmempp);
}



sword SDO_OCIInitialize(ub4 mode, dvoid *ctxp, 

						dvoid *(*malocfp)(dvoid *ctxp, size_t size),

						dvoid *(*ralocfp)(dvoid *ctxp, dvoid *memptr, size_t newsize),

						void (*mfreefp)(dvoid *ctxp, dvoid *memptr))

{

	return OCIInitialize(mode, ctxp, malocfp, ralocfp, mfreefp);

}



sword SDO_OCIHandleAlloc(CONST dvoid *parenth, dvoid **hndlpp, ub4 type, 

						 size_t xtramem_sz, dvoid **usrmempp)

{

	//aloca todas as handles do ambiente passado como parametro
	return OCIHandleAlloc(parenth, hndlpp, type, xtramem_sz, usrmempp);

}



sword SDO_OCIHandleFree(dvoid *hndlp, ub4 type)

{

	return OCIHandleFree(hndlp, type);

}



sword SDO_OCIDescriptorAlloc(CONST dvoid *parenth, dvoid **descpp, ub4 type,

							 size_t xtramem_sz, dvoid **usrmempp)

{

	return OCIDescriptorAlloc(parenth, descpp, type, xtramem_sz, usrmempp);

}



sword SDO_OCIDescriptorFree(dvoid *descp, ub4 type)

{

	return OCIDescriptorFree(descp, type);

}



sword SDO_OCIEnvInit(OCIEnv **envp, ub4 mode, size_t xtramem_sz, 

					 dvoid **usrmempp)

{

	return OCIEnvInit(envp, mode, xtramem_sz, usrmempp);

}



sword SDO_OCIServerAttach(OCIServer *srvhp, OCIError *errhp,

						  CONST text *dblink, sb4 dblink_len, ub4 mode)

{

	return OCIServerAttach(srvhp, errhp, dblink, dblink_len, mode);

}



sword SDO_OCIServerDetach(OCIServer *srvhp, OCIError *errhp, ub4 mode)

{

	return OCIServerDetach(srvhp, errhp, mode);

}



sword SDO_OCISessionBegin(OCISvcCtx *svchp, OCIError *errhp, 

						  OCISession *usrhp, ub4 credt, ub4 mode)

{

	return OCISessionBegin(svchp, errhp, usrhp, credt, mode);

}



sword SDO_OCISessionEnd(OCISvcCtx *svchp, OCIError *errhp, 

						OCISession *usrhp, ub4 mode)

{

	return OCISessionEnd(svchp, errhp, usrhp, mode);

}



sword SDO_OCILogon(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp, 

				   CONST text *username, ub4 uname_len, CONST text *password, 

				   ub4 passwd_len, CONST text *dbname, ub4 dbname_len)

{

	return OCILogon(envhp, errhp, svchp, username, uname_len, password, 

				    passwd_len, dbname, dbname_len);

}



sword SDO_OCILogoff(OCISvcCtx *svchp, OCIError *errhp)

{

	return OCILogoff(svchp, errhp);

}



sword SDO_OCIPasswordChange(OCISvcCtx *svchp, OCIError *errhp, 

							CONST text *user_name, ub4 usernm_len, 

							CONST text *opasswd, ub4 opasswd_len, 

							CONST text *npasswd, ub4 npasswd_len, ub4 mode)

{

	return OCIPasswordChange(svchp, errhp, user_name, usernm_len, 

							 opasswd, opasswd_len, npasswd, npasswd_len, mode);

}



sword SDO_OCIStmtPrepare(OCISvcCtx * /* svchp */, OCIStmt *stmtp, OCIError *errhp, CONST OraText *stmt,

						 ub4 stmt_len, CONST OraText * /* key */, ub4 /* keylen */, ub4 language, ub4 mode)

{
	//return OCIStmtPrepare2 (svchp, stmtp, errhp, stmt, stmt_len, key, keylen, language, mode );
	
	return OCIStmtPrepare(stmtp, errhp, stmt, stmt_len, language, mode);
}



sword SDO_OCIBindByPos(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,

					   ub4 position, dvoid *valuep, sb4 value_sz,

					   ub2 dty, dvoid *indp, ub2 *alenp, ub2 *rcodep,

					   ub4 maxarr_len, ub4 *curelep, ub4 mode)

{

	return OCIBindByPos(stmtp, bindp, errhp, position, valuep, value_sz,

					    dty, indp, alenp, rcodep, maxarr_len, curelep, mode);

}



sword SDO_OCIBindByName(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,

						CONST text *placeholder, sb4 placeh_len, 

						dvoid *valuep, sb4 value_sz, ub2 dty, 

						dvoid *indp, ub2 *alenp, ub2 *rcodep, 

						ub4 maxarr_len, ub4 *curelep, ub4 mode)

{

	return OCIBindByName(stmtp, bindp, errhp, placeholder, placeh_len, 

						 valuep, value_sz, dty, indp, alenp, rcodep, 

						 maxarr_len, curelep, mode);

}



sword SDO_OCIBindObject(OCIBind *bindp, OCIError *errhp, CONST OCIType *type, 

						dvoid **pgvpp, ub4 *pvszsp, dvoid **indpp, ub4 *indszp)

{

	return OCIBindObject(bindp, errhp, type, pgvpp, pvszsp, indpp, indszp);

}



sword SDO_OCIBindDynamic(OCIBind *bindp, OCIError *errhp, dvoid *ictxp,

						 OCICallbackInBind icbfp, dvoid *octxp,

						 OCICallbackOutBind ocbfp)

{

	return OCIBindDynamic(bindp, errhp, ictxp, icbfp, octxp, ocbfp);

}



sword SDO_OCIBindArrayOfStruct(OCIBind *bindp, OCIError *errhp, 

                               ub4 pvskip, ub4 indskip, ub4 alskip, ub4 rcskip)

{

	return OCIBindArrayOfStruct(bindp, errhp, pvskip, indskip, alskip, rcskip);

}



sword SDO_OCIStmtGetPieceInfo(OCIStmt *stmtp, OCIError *errhp, 

                              dvoid **hndlpp, ub4 *typep, ub1 *in_outp, 

							  ub4 *iterp, ub4 *idxp, ub1 *piecep)

{

	return OCIStmtGetPieceInfo(stmtp, errhp, hndlpp, typep, in_outp, iterp, 

							   idxp, piecep);

}



sword SDO_OCIStmtSetPieceInfo(dvoid *hndlp, ub4 type, OCIError *errhp, 

                              CONST dvoid *bufp, ub4 *alenp, ub1 piece, 

                              CONST dvoid *indp, ub2 *rcodep)

{

	return OCIStmtSetPieceInfo(hndlp, type, errhp, bufp, alenp, piece, 

							   indp, rcodep);

}



sword SDO_OCIStmtExecute(OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp, 

						 ub4 iters, ub4 rowoff, CONST OCISnapshot *snap_in, 

						 OCISnapshot *snap_out, ub4 mode)

{

	return OCIStmtExecute(svchp, stmtp, errhp, iters, rowoff, snap_in, 

						  snap_out, mode);

}



sword SDO_OCIDefineByPos(OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,

						 ub4 position, dvoid *valuep, sb4 value_sz, ub2 dty,

						 dvoid *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode)

{

	return OCIDefineByPos(stmtp, defnp, errhp, position, valuep, value_sz, dty,

						  indp, rlenp, rcodep, mode);

}



sword SDO_OCIDefineObject(OCIDefine *defnp, OCIError *errhp, 

                          CONST OCIType *type, dvoid **pgvpp, 

                          ub4 *pvszsp, dvoid **indpp, ub4 *indszp)

{	

	return OCIDefineObject(defnp, errhp, type, pgvpp, pvszsp, indpp, indszp);

}



sword SDO_OCIDefineDynamic(OCIDefine *defnp, OCIError *errhp, dvoid *octxp,

                           OCICallbackDefine ocbfp)

{

	return OCIDefineDynamic(defnp, errhp, octxp, ocbfp);

}



sword SDO_OCIDefineArrayOfStruct(OCIDefine *defnp, OCIError *errhp, ub4 pvskip,

                                 ub4 indskip, ub4 rlskip, ub4 rcskip)

{

	return OCIDefineArrayOfStruct(defnp, errhp, pvskip, indskip, rlskip, rcskip);

}



sword SDO_OCIStmtFetch(OCIStmt *stmtp, OCIError *errhp, ub4 nrows, 

                       ub2 orientation, ub4 mode)

{

	return OCIStmtFetch(stmtp, errhp, nrows, orientation, mode);

}



sword SDO_OCIStmtGetBindInfo(OCIStmt *stmtp, OCIError *errhp, ub4 size, 

                             ub4 startloc, sb4 *found, text *bvnp[], ub1 bvnl[],

                             text *invp[], ub1 inpl[], ub1 dupl[], OCIBind *hndl[])

{

	return OCIStmtGetBindInfo(stmtp, errhp, size, startloc, found, bvnp, bvnl,

                              invp, inpl, dupl, hndl);

}



sword SDO_OCIDescribeAny(OCISvcCtx *svchp, OCIError *errhp, dvoid *objptr, 

                         ub4 objnm_len, ub1 objptr_typ, ub1 info_level,

						 ub1 objtyp, OCIDescribe *dschp)

{

	return OCIDescribeAny(svchp, errhp, objptr, objnm_len, objptr_typ, info_level,

						  objtyp, dschp);

}



sword SDO_OCIParamGet(CONST dvoid *hndlp, ub4 htype, OCIError *errhp, 

                      dvoid **parmdpp, ub4 pos)

{

	return OCIParamGet(hndlp, htype, errhp, parmdpp, pos);

}



sword SDO_OCIParamSet(dvoid *hdlp, ub4 htyp, OCIError *errhp, CONST dvoid *dscp,

                      ub4 dtyp, ub4 pos)

{

	return OCIParamSet(hdlp, htyp, errhp, dscp, dtyp, pos);

}



sword SDO_OCITransStart(OCISvcCtx *svchp, OCIError *errhp, uword timeout, ub4 flags)

{

	return OCITransStart(svchp, errhp, timeout, flags);

}



sword SDO_OCITransDetach(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)

{

	return OCITransDetach(svchp, errhp, flags);

}



sword SDO_OCITransCommit(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)

{

	return OCITransCommit(svchp, errhp, flags);

}



sword SDO_OCITransRollback(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)

{

	return OCITransRollback(svchp, errhp, flags);

}



sword SDO_OCITransPrepare(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)

{

	return OCITransPrepare(svchp, errhp, flags);

}



sword SDO_OCITransForget(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)

{

	return OCITransForget(svchp, errhp, flags);

}



sword SDO_OCIErrorGet(dvoid *hndlp, ub4 recordno, text *sqlstate,

                      sb4 *errcodep, text *bufp, ub4 bufsiz, ub4 type)

{

	return OCIErrorGet(hndlp, recordno, sqlstate, errcodep, bufp, bufsiz, type);

}



sword SDO_OCILobAppend(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *dst_locp,

                       OCILobLocator *src_locp)

{

	return OCILobAppend(svchp, errhp, dst_locp, src_locp);

}



sword SDO_OCILobAssign(OCIEnv *envhp, OCIError *errhp, CONST OCILobLocator *src_locp, 

                       OCILobLocator **dst_locpp)

{

	return OCILobAssign(envhp, errhp, src_locp, dst_locpp);

}



sword SDO_OCILobCharSetForm(OCIEnv *envhp, OCIError *errhp, 

							CONST OCILobLocator *locp, ub1 *csfrm)

{

	return OCILobCharSetForm(envhp, errhp, locp, csfrm);

}



sword SDO_OCILobCharSetId(OCIEnv *envhp, OCIError *errhp, 

						  CONST OCILobLocator *locp, ub2 *csid)

{

	return OCILobCharSetId(envhp, errhp, locp, csid);

}



sword SDO_OCILobCopy(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *dst_locp,

                     OCILobLocator *src_locp, ub4 amount, ub4 dst_offset, 

                     ub4 src_offset)

{

	return OCILobCopy(svchp, errhp, dst_locp, src_locp, amount, dst_offset, 

                      src_offset);

}



sword SDO_OCILobDisableBuffering(OCISvcCtx *svchp, OCIError *errhp,

								 OCILobLocator *locp)

{

	return OCILobDisableBuffering(svchp, errhp, locp);

}



sword SDO_OCILobEnableBuffering(OCISvcCtx *svchp, OCIError *errhp,

								OCILobLocator *locp)

{

	return OCILobEnableBuffering(svchp, errhp, locp);

}



sword SDO_OCILobErase(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                      ub4 *amount, ub4 offset)

{

	return OCILobErase(svchp, errhp, locp, amount, offset);

}



sword SDO_OCILobFileClose(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep)

{

	return OCILobFileClose(svchp, errhp, filep);

}



sword SDO_OCILobFileCloseAll(OCISvcCtx *svchp, OCIError *errhp)

{

	return OCILobFileCloseAll(svchp, errhp);

}



sword SDO_OCILobFileExists(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,

						   boolean *flag)

{

	return OCILobFileExists(svchp, errhp, filep, flag);

}



sword SDO_OCILobFileGetName(OCIEnv *envhp, OCIError *errhp, 

							CONST OCILobLocator *filep, text *dir_alias, 

							ub2 *d_length, text *filename, ub2 *f_length)

{

	return OCILobFileGetName(envhp, errhp, filep, dir_alias, d_length, 

							 filename, f_length);

}



sword SDO_OCILobFileIsOpen(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,

                           boolean *flag)

{

	return OCILobFileIsOpen(svchp, errhp, filep, flag);

}



sword SDO_OCILobFileOpen(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,

                         ub1 mode)

{

	return OCILobFileOpen(svchp, errhp, filep, mode);

}



sword SDO_OCILobFileSetName(OCIEnv *envhp, OCIError *errhp, OCILobLocator **filepp, 

                            CONST text *dir_alias, ub2 d_length, 

                            CONST text *filename, ub2 f_length)

{

	return OCILobFileSetName(envhp, errhp, filepp, dir_alias, d_length, 

                             filename, f_length);

}



sword SDO_OCILobFlushBuffer(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                            ub4 flag)

{

	return OCILobFlushBuffer(svchp, errhp, locp, flag);

}



sword SDO_OCILobGetLength(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                          ub4 *lenp)

{

	return OCILobGetLength(svchp, errhp, locp, lenp);

}



sword SDO_OCILobIsEqual(OCIEnv *envhp, CONST OCILobLocator *x, 

						CONST OCILobLocator *y, boolean *is_equal)

{

	return OCILobIsEqual(envhp, x, y, is_equal);

}



sword SDO_OCILobLoadFromFile(OCISvcCtx *svchp, OCIError *errhp, 

							 OCILobLocator *dst_locp, OCILobLocator *src_filep, 

                             ub4 amount, ub4 dst_offset, ub4 src_offset)

{

	return OCILobLoadFromFile(svchp, errhp, dst_locp, src_filep, amount, 

							  dst_offset, src_offset);

}



sword SDO_OCILobLocatorIsInit(OCIEnv *envhp, OCIError *errhp, 

							  CONST OCILobLocator *locp, boolean *is_initialized)

{

	return OCILobLocatorIsInit(envhp, errhp, locp, is_initialized);

}



sword SDO_OCILobRead(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                     ub4 *amtp, ub4 offset, dvoid *bufp, ub4 bufl, 

                     dvoid *ctxp, sb4 (*cbfp)(dvoid *ctxp, CONST dvoid *bufp, 

					 ub4 len, ub1 piece), ub2 csid, ub1 csfrm)

{

	return OCILobRead(svchp, errhp, locp, amtp, offset, bufp, bufl, 

                      ctxp, cbfp, csid, csfrm);

}



sword SDO_OCILobTrim(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                     ub4 newlen)

{

	return OCILobTrim(svchp, errhp, locp, newlen);

}



sword SDO_OCILobWrite(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,

                      ub4 *amtp, ub4 offset, dvoid *bufp, ub4 buflen, ub1 piece, 

					  dvoid *ctxp, sb4 (*cbfp)(dvoid *ctxp, dvoid *bufp, ub4 *len, 

					  ub1 *piece), ub2 csid, ub1 csfrm)

{

	return OCILobWrite(svchp, errhp, locp, amtp, offset, bufp, buflen, piece, 

					   ctxp, cbfp, csid, csfrm);

}



sword SDO_OCIBreak(dvoid *hndlp, OCIError *errhp)

{

	return OCIBreak(hndlp, errhp);

}



sword SDO_OCIServerVersion(dvoid *hndlp, OCIError *errhp, text *bufp, ub4 bufsz,

                           ub1 hndltype)

{

	return OCIServerVersion(hndlp, errhp, bufp, bufsz, hndltype);

}



sword SDO_OCIAttrGet(CONST dvoid *trgthndlp, ub4 trghndltyp, dvoid *attributep, 

					 ub4 *sizep, ub4 attrtype, OCIError *errhp)

{

	return OCIAttrGet(trgthndlp, trghndltyp, attributep, sizep, attrtype, errhp);

}



sword SDO_OCIAttrSet(dvoid *trgthndlp, ub4 trghndltyp, dvoid *attributep,

					 ub4 size, ub4 attrtype, OCIError *errhp)

{

	return OCIAttrSet(trgthndlp, trghndltyp, attributep, size, attrtype, errhp);

}



sword SDO_OCISvcCtxToLda(OCISvcCtx *svchp, OCIError *errhp, Lda_Def *ldap)

{

	return OCISvcCtxToLda(svchp, errhp, ldap);

}



sword SDO_OCILdaToSvcCtx(OCISvcCtx **svchpp, OCIError *errhp, Lda_Def *ldap)

{

	return OCILdaToSvcCtx(svchpp, errhp, ldap);

}



sword SDO_OCIResultSetToStmt(OCIResult *rsetdp, OCIError *errhp)

{

	return OCIResultSetToStmt(rsetdp, errhp);

}





/********************************************************************************/

/* Part II -- OCI navigational interfaces                                       */

/********************************************************************************/

sword SDO_OCIObjectNew(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

					   OCITypeCode typecode, OCIType *tdo, dvoid *table, 

					   OCIDuration duration, boolean value, dvoid **instance)

{

	return OCIObjectNew(env, err, svc, typecode, tdo, table, duration, 

						value, instance);

}



sword SDO_OCIObjectPin(OCIEnv *env, OCIError *err, OCIRef *object_ref, 

					   OCIComplexObject *corhdl, OCIPinOpt pin_option, 

					   OCIDuration pin_duration, OCILockOpt lock_option, 

					   dvoid **object)

{

	return OCIObjectPin(env, err, object_ref, corhdl, pin_option, 

					    pin_duration, lock_option, object);

}



sword SDO_OCIObjectUnpin(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectUnpin(env, err, object);

}



sword SDO_OCIObjectPinCountReset(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectPinCountReset(env, err, object);

}



sword SDO_OCIObjectLock(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectLock(env, err, object);

}



sword SDO_OCIObjectMarkUpdate(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectMarkUpdate(env, err, object);

}



sword SDO_OCIObjectUnmark(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectUnmark(env, err, object);

}



sword SDO_OCIObjectUnmarkByRef(OCIEnv *env, OCIError *err, OCIRef *ref)

{

	return OCIObjectUnmarkByRef(env, err, ref);

}



sword SDO_OCIObjectFree(OCIEnv *env, OCIError *err, dvoid *instance, 

                        ub2 flags)

{

	return OCIObjectFree(env, err, instance, flags);

}



sword SDO_OCIObjectMarkDeleteByRef(OCIEnv *env, OCIError *err, OCIRef *object_ref)

{

	return OCIObjectMarkDeleteByRef(env, err, object_ref);

}



sword SDO_OCIObjectMarkDelete(OCIEnv *env, OCIError *err, dvoid *instance)

{

	return OCIObjectMarkDelete(env, err, instance);

}



sword SDO_OCIObjectFlush(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectFlush(env, err, object);

}



sword SDO_OCIObjectRefresh(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectRefresh(env, err, object);

}



sword SDO_OCIObjectCopy(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        dvoid *source, dvoid *null_source, dvoid *target, 

						dvoid *null_target, OCIType *tdo, OCIDuration duration, 

						ub1 option)

{

	return OCIObjectCopy(env, err, svc, source, null_source, target, 

						 null_target, tdo, duration, option);

}



sword SDO_OCIObjectGetTypeRef(OCIEnv *env, OCIError *err, dvoid *instance, 

                              OCIRef *type_ref)

{

	return OCIObjectGetTypeRef(env, err, instance, type_ref);

}



sword SDO_OCIObjectGetObjectRef(OCIEnv *env, OCIError *err, dvoid *object, 

                                OCIRef *object_ref)

{

	return OCIObjectGetObjectRef(env, err, object, object_ref);

}



sword SDO_OCIObjectGetInd(OCIEnv *env, OCIError *err, dvoid *instance, 

                          dvoid **null_struct)

{

	return OCIObjectGetInd(env, err, instance, null_struct);

}



sword SDO_OCIObjectExists(OCIEnv *env, OCIError *err, dvoid *ins, boolean *exist)

{

	return OCIObjectExists(env, err, ins, exist);

}



sword SDO_OCIObjectGetProperty(OCIEnv *envh, OCIError *errh, CONST dvoid *obj, 

							   OCIObjectPropId propertyId, dvoid *property, 

							   ub4 *size)

{

	return OCIObjectGetProperty(envh, errh, obj, propertyId, property, size);

}



sword SDO_OCIObjectIsLocked(OCIEnv *env, OCIError *err, dvoid *ins, boolean *lock)

{

	return OCIObjectIsLocked(env, err, ins, lock);

}



sword SDO_OCIObjectIsDirty(OCIEnv *env, OCIError *err, dvoid *ins, boolean *dirty)

{

	return OCIObjectIsDirty(env, err, ins, dirty);

}



sword SDO_OCIObjectPinTable(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

							CONST text *schema_name, ub4 s_n_length, 

							CONST text *object_name, ub4 o_n_length, 

							CONST OCIRef *scope_obj_ref, OCIDuration pin_duration, 

							dvoid** object)

{

	return OCIObjectPinTable(env, err, svc, schema_name, s_n_length, 

							 object_name, o_n_length, scope_obj_ref, pin_duration, 

							 object);

}



sword SDO_OCIObjectArrayPin(OCIEnv *env, OCIError *err, OCIRef **ref_array, 

							ub4 array_size, OCIComplexObject **cor_array,

							ub4 cor_array_size, OCIPinOpt pin_option, 

							OCIDuration pin_duration, OCILockOpt lock, 

							dvoid **obj_array, ub4 *pos)

{

	return OCIObjectArrayPin(env, err, ref_array, array_size, cor_array,

							 cor_array_size, pin_option, pin_duration, lock, 

							 obj_array, pos);

}



sword SDO_OCICacheFlush(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

						dvoid *context, OCIRef *(*get)(dvoid *context, ub1 *last),

						OCIRef **ref)

{

	return OCICacheFlush(env, err, svc, context, get, ref);

}



sword SDO_OCICacheRefresh(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

						  OCIRefreshOpt option, dvoid *context,

						  OCIRef *(*get)(dvoid *context), OCIRef **ref)

{

	return OCICacheRefresh(env, err, svc, option, context, get, ref);

}



sword SDO_OCICacheUnpin(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc)

{

	return OCICacheUnpin(env, err, svc);

}



sword SDO_OCICacheFree(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc)

{

	return OCICacheFree(env, err, svc);

} 



sword SDO_OCICacheUnmark(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc)

{

	return OCICacheUnmark(env, err, svc);

}



sword SDO_OCIDurationBegin(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                           OCIDuration parent, OCIDuration *dur)

{

	return OCIDurationBegin(env, err, svc, parent, dur);

}



sword SDO_OCIDurationEnd(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                         OCIDuration duration)

{

	return OCIDurationEnd(env, err, svc, duration);

}



sword SDO_OCIDurationGetParent(OCIEnv *env, OCIError *err, 

                               OCIDuration duration, OCIDuration *parent)

{

	return OCIDurationGetParent(env, err, duration, parent);

}



sword SDO_OCIObjectAlwaysLatest(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectAlwaysLatest(env, err, object);

}



sword SDO_OCIObjectNotAlwaysLatest(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectNotAlwaysLatest(env, err, object);

}



sword SDO_OCIObjectFlushRefresh(OCIEnv *env, OCIError *err, dvoid *object)

{

	return OCIObjectFlushRefresh(env, err, object);

}



sword SDO_OCIObjectIsLoaded(OCIEnv *env, OCIError *err, dvoid *ins, 

							boolean *load)

{

	return OCIObjectIsLoaded(env, err, ins, load);

}



sword SDO_OCIObjectIsDirtied(OCIEnv *env, OCIError *err, dvoid *ins,

							 boolean *dirty)

{

	return OCIObjectIsDirtied(env, err, ins, dirty);

}



sword SDO_OCICacheGetObjects(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

							 OCIObjectProperty property, dvoid *client_context,

							 void (*client_callback)(dvoid *client_context,

							 dvoid *object))

{

	return OCICacheGetObjects(env, err, svc, property, client_context,

							  client_callback);

}



sword SDO_OCICacheRegister(OCIEnv *env, OCIError *err, OCIObjectEvent event,

						   dvoid *client_context, void (*client_callback)(

						   dvoid *client_context, OCIObjectEvent event,

						   dvoid *object))

{

	return OCICacheRegister(env, err, event, client_context, client_callback);

}



sword SDO_OCICacheFlushRefresh(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

							   dvoid *context, OCIRef *(*get)(dvoid *context, 

							   ub1 *last), OCIRef **ref)

{

	return OCICacheFlushRefresh(env, err, svc, context, get, ref);

}





/********************************************************************************/

/* Part III -- OCI primitive OTS interfaces                                     */

/********************************************************************************/

void SDO_OCINumberSetZero(OCIError *err, OCINumber *num)

{

	OCINumberSetZero(err, num);

}



sword SDO_OCINumberAdd(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberAdd(err, number1, number2, result);

}



sword SDO_OCINumberSub(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberSub(err, number1, number2, result);

}



sword SDO_OCINumberMul(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberMul(err, number1, number2, result);

}



sword SDO_OCINumberDiv(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberDiv(err, number1, number2, result);

}



sword SDO_OCINumberMod(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberMod(err, number1, number2, result);

}



sword SDO_OCINumberIntPower(OCIError *err, CONST OCINumber *base,

                            CONST sword exp, OCINumber *result)

{

	return OCINumberIntPower(err, base, exp, result);

}



sword SDO_OCINumberNeg(OCIError *err, CONST OCINumber *number,

                       OCINumber *result)

{

	return OCINumberNeg(err, number, result);

}



sword SDO_OCINumberToText(OCIError *err, CONST OCINumber *number, 

                          CONST text *fmt, ub4 fmt_length,

                          CONST text *nls_params, ub4 nls_p_length,

                          ub4 *buf_size, text *buf)

{

	return OCINumberToText(err, number, fmt, fmt_length, nls_params, 

						   nls_p_length, buf_size, buf);

}



sword SDO_OCINumberFromText(OCIError *err, CONST text *str, ub4 str_length, 

							CONST text *fmt, ub4 fmt_length, 

							CONST text *nls_params, ub4 nls_p_length, 

							OCINumber *number)

{

	return OCINumberFromText(err, str, str_length, fmt, fmt_length, 

							 nls_params, nls_p_length, number);

}



sword SDO_OCINumberToInt(OCIError *err, CONST OCINumber *number,

                         uword rsl_length, uword rsl_flag, dvoid *rsl)

{

	return OCINumberToInt(err, number, rsl_length, rsl_flag, rsl);

}



sword SDO_OCINumberFromInt(OCIError *err, CONST dvoid *inum, uword inum_length,

						   uword inum_s_flag, OCINumber *number)

{

	return OCINumberFromInt(err, inum, inum_length, inum_s_flag, number);

}



sword SDO_OCINumberToReal(OCIError *err, CONST OCINumber *number,

                          uword rsl_length, dvoid *rsl)

{

	return OCINumberToReal(err, number, rsl_length, rsl);

}



sword SDO_OCINumberFromReal(OCIError *err, CONST dvoid *rnum,

                            uword rnum_length, OCINumber *number)

{

	return OCINumberFromReal(err, rnum, rnum_length, number);

}



sword SDO_OCINumberCmp(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, sword *result)

{

	return OCINumberCmp(err, number1, number2, result);

}



sword SDO_OCINumberSign(OCIError *err, CONST OCINumber *number,

                        sword *result)

{

	return OCINumberSign(err, number, result);

}



sword SDO_OCINumberIsZero(OCIError *err, CONST OCINumber *number,

                          boolean *result)

{

	return OCINumberIsZero(err, number, result);

}



sword SDO_OCINumberAssign(OCIError *err, CONST OCINumber *from,

                          OCINumber *to)

{

	return OCINumberAssign(err, from, to);

}



sword SDO_OCINumberAbs(OCIError *err, CONST OCINumber *number,

                       OCINumber *result)

{

	return OCINumberAbs(err, number, result);

}



sword SDO_OCINumberCeil(OCIError *err, CONST OCINumber *number,

                        OCINumber *result)

{

	return OCINumberCeil(err, number, result);

}



sword SDO_OCINumberFloor(OCIError *err, CONST OCINumber *number, 

                         OCINumber *result)

{

	return OCINumberFloor(err, number, result);

}



sword SDO_OCINumberSqrt(OCIError *err, CONST OCINumber *number, 

                        OCINumber *result)

{

	return OCINumberSqrt(err, number, result);

}



sword SDO_OCINumberTrunc(OCIError *err, CONST OCINumber *number, 

                         sword decplace, OCINumber *result)

{

	return OCINumberTrunc(err, number, decplace, result);

}



sword SDO_OCINumberPower(OCIError *err, CONST OCINumber *base, 

                         CONST OCINumber *number, OCINumber *result)

{

	return OCINumberPower(err, base, number, result);

}



sword SDO_OCINumberRound(OCIError *err, CONST OCINumber *number,

                         sword decplace, OCINumber *result)

{

	return OCINumberRound(err, number, decplace, result);

}



sword SDO_OCINumberSin(OCIError *err, CONST OCINumber *number,

                       OCINumber *result)

{

	return OCINumberSin(err, number, result);

}



sword SDO_OCINumberArcSin(OCIError *err, CONST OCINumber *number,

                          OCINumber *result)

{

	return OCINumberArcSin(err, number, result);

}



sword SDO_OCINumberHypSin(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result)

{

	return OCINumberHypSin(err, number, result);

}



sword SDO_OCINumberCos(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result)

{

	return OCINumberCos(err, number, result);

}



sword SDO_OCINumberArcCos(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result)

{

	return OCINumberArcCos(err, number, result);

}



sword SDO_OCINumberHypCos(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result)

{

	return OCINumberHypCos(err, number, result);

}



sword SDO_OCINumberTan(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result)

{

	return OCINumberTan(err, number, result);

}



sword SDO_OCINumberArcTan(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result)

{

	return OCINumberArcTan(err, number, result);

}



sword SDO_OCINumberArcTan2(OCIError *err, CONST OCINumber *number1, 

                           CONST OCINumber *number2, OCINumber *result)

{

	return OCINumberArcTan2(err, number1, number2, result);

}



sword SDO_OCINumberHypTan(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result)

{

	return OCINumberHypTan(err, number, result);

}



sword SDO_OCINumberExp(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result)

{

	return OCINumberExp(err, number, result);

}



sword SDO_OCINumberLn(OCIError *err, CONST OCINumber *number, 

                      OCINumber *result)

{

	return OCINumberLn(err, number, result);

}



sword SDO_OCINumberLog(OCIError *err, CONST OCINumber *base, 

                       CONST OCINumber *number, OCINumber *result)

{

	return OCINumberLog(err, base, number, result);

}



sword SDO_OCIDateAssign(OCIError *err, CONST OCIDate *from, OCIDate *to)

{

	return OCIDateAssign(err, from, to);

}



sword SDO_OCIDateToText(OCIError *err, CONST OCIDate *date, 

                        CONST text *fmt, ub1 fmt_length, 

                        CONST text *lang_name, ub4 lang_length, 

                        ub4 *buf_size, text *buf)

{

	return OCIDateToText(err, date, fmt, fmt_length, lang_name, lang_length, 

						 buf_size, buf);

}



sword SDO_OCIDateFromText(OCIError *err, CONST text *date_str, 

						  ub4 d_str_length, CONST text *fmt, ub1 fmt_length, 

						  CONST text *lang_name, ub4 lang_length, 

						  OCIDate *date)

{

	return OCIDateFromText(err, date_str, d_str_length, fmt, fmt_length, 

						   lang_name, lang_length, date);

}



sword SDO_OCIDateCompare(OCIError *err, CONST OCIDate *date1, 

						 CONST OCIDate *date2, sword *result)

{

	return OCIDateCompare(err, date1, date2, result);

}



sword SDO_OCIDateAddMonths(OCIError *err, CONST OCIDate *date, sb4 num_months,

                           OCIDate *result)

{

	return OCIDateAddMonths(err, date, num_months, result);

}



sword SDO_OCIDateAddDays(OCIError *err, CONST OCIDate *date, sb4 num_days,

                         OCIDate *result)

{

	return OCIDateAddDays(err, date, num_days, result);

}



sword SDO_OCIDateLastDay(OCIError *err, CONST OCIDate *date, 

                         OCIDate *last_day)

{

	return OCIDateLastDay(err, date, last_day);

}



sword SDO_OCIDateDaysBetween(OCIError *err, CONST OCIDate *date1, 

                             CONST OCIDate *date2, sb4 *num_days)

{

	return OCIDateDaysBetween(err, date1, date2, num_days);

}



sword SDO_OCIDateZoneToZone(OCIError *err, CONST OCIDate *date1,

                            CONST text *zon1, ub4 zon1_length, 

							CONST text *zon2, ub4 zon2_length, OCIDate *date2)

{

	return OCIDateZoneToZone(err, date1, zon1, zon1_length, zon2, 

							 zon2_length, date2);

}



sword SDO_OCIDateNextDay(OCIError *err, CONST OCIDate *date, CONST text *day_p, 

                         ub4 day_length, OCIDate *next_day)

{

	return OCIDateNextDay(err, date, day_p, day_length, next_day);

}





sword SDO_OCIDateCheck(OCIError *err, CONST OCIDate *date, uword *valid)

{

	return OCIDateCheck(err, date, valid);

}



sword SDO_OCIDateSysDate(OCIError *err, OCIDate *sys_date)

{

	return OCIDateSysDate(err, sys_date);

}



sword SDO_OCIStringAssign(OCIEnv *env, OCIError *err, CONST OCIString *rhs, 

                          OCIString **lhs)

{

	return OCIStringAssign(env, err, rhs, lhs);

}



sword SDO_OCIStringAssignText(OCIEnv *env, OCIError *err, CONST text *rhs, 

                              ub4 rhs_len, OCIString **lhs)

{

	return OCIStringAssignText(env, err, rhs, rhs_len, lhs);

}



sword SDO_OCIStringResize(OCIEnv *env, OCIError *err, ub4 new_size, 

                          OCIString **str)

{

	return OCIStringResize(env, err, new_size, str);

}



ub4 SDO_OCIStringSize(OCIEnv *env, CONST OCIString *vs)

{

	return OCIStringSize(env, vs);

}



text *SDO_OCIStringPtr(OCIEnv *env, CONST OCIString *vs)

{

	return OCIStringPtr(env, vs);

}



sword SDO_OCIStringAllocSize(OCIEnv *env, OCIError *err, CONST OCIString *vs, 

                             ub4 *allocsize)

{

	return OCIStringAllocSize(env, err, vs, allocsize);

}



sword SDO_OCIRawAssignRaw(OCIEnv *env, OCIError *err, CONST OCIRaw *rhs, 

                          OCIRaw **lhs)

{

	return OCIRawAssignRaw(env, err, rhs, lhs);

}



sword SDO_OCIRawAssignBytes(OCIEnv *env, OCIError *err, CONST ub1 *rhs, 

                            ub4 rhs_len, OCIRaw **lhs)

{

	return OCIRawAssignBytes(env, err, rhs, rhs_len, lhs);

}



sword SDO_OCIRawResize(OCIEnv *env, OCIError *err, ub4 new_size, OCIRaw **raw)

{

	return OCIRawResize(env, err, new_size, raw);

}



ub4 SDO_OCIRawSize(OCIEnv *env, CONST OCIRaw *raw)

{

	return OCIRawSize(env, raw);

}



ub1 *SDO_OCIRawPtr(OCIEnv *env, CONST OCIRaw *raw)

{

	return OCIRawPtr(env, raw);

}



sword SDO_OCIRawAllocSize(OCIEnv *env, OCIError *err, CONST OCIRaw *raw,

						  ub4 *allocsize)

{

	return OCIRawAllocSize(env, err, raw, allocsize);

}



void SDO_OCIRefClear(OCIEnv *env, OCIRef *ref)

{

	OCIRefClear(env, ref);

}



sword SDO_OCIRefAssign(OCIEnv *env, OCIError *err, CONST OCIRef *source, 

                       OCIRef **target)

{

	return OCIRefAssign(env, err, source, target);

}



boolean SDO_OCIRefIsEqual(OCIEnv *env, CONST OCIRef *x, CONST OCIRef *y)

{

	return OCIRefIsEqual(env, x, y);

}



boolean SDO_OCIRefIsNull(OCIEnv *env, CONST OCIRef *ref)

{

	return OCIRefIsNull(env, ref);

}



ub4 SDO_OCIRefHexSize(OCIEnv *env, CONST OCIRef *ref)

{

	return OCIRefHexSize(env, ref);

}



sword SDO_OCIRefFromHex(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        CONST text *hex, ub4 length, OCIRef **ref)

{

	return OCIRefFromHex(env, err, svc, hex, length, ref);

}



sword SDO_OCIRefToHex(OCIEnv *env, OCIError *err, CONST OCIRef *ref, 

                      text *hex, ub4 *hex_length)

{

	return OCIRefToHex(env, err, ref, hex, hex_length);

}



sword SDO_OCICollSize(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

					  sb4 *size)

{

	return OCICollSize(env, err, coll, size);

}



sb4 SDO_OCICollMax(OCIEnv *env, CONST OCIColl *coll)

{

	return OCICollMax(env, coll);

}



sword SDO_OCICollGetElem(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                         sb4 index, boolean *exists, dvoid **elem, 

                         dvoid **elemind)

{

	return OCICollGetElem(env, err, coll, index, exists, elem, elemind);

}



sword SDO_OCICollAssignElem(OCIEnv *env, OCIError *err, sb4 index, 

                            CONST dvoid *elem, 

                            CONST dvoid *elemind, OCIColl *coll)

{

	return OCICollAssignElem(env, err, index, elem, elemind, coll);

}



sword SDO_OCICollAssign(OCIEnv *env, OCIError *err, CONST OCIColl *rhs, 

                        OCIColl *lhs)

{

	return OCICollAssign(env, err, rhs, lhs);

}



sword SDO_OCICollAppend(OCIEnv *env, OCIError *err, CONST dvoid *elem, 

                        CONST dvoid *elemind, OCIColl *coll)

{

	return OCICollAppend(env, err, elem, elemind, coll);

}



sword SDO_OCICollTrim(OCIEnv *env, OCIError *err, sb4 trim_num, 

                      OCIColl *coll)

{

	return OCICollTrim(env, err, trim_num, coll);

}



sword SDO_OCIIterCreate(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                        OCIIter **itr)

{

	return OCIIterCreate(env, err, coll, itr);

}



sword SDO_OCIIterDelete(OCIEnv *env, OCIError *err, OCIIter **itr)

{

	return OCIIterDelete(env, err, itr);

}



sword SDO_OCIIterInit(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                      OCIIter *itr)

{

	return OCIIterInit(env, err, coll, itr);

}



sword SDO_OCIIterGetCurrent(OCIEnv *env, OCIError *err, CONST OCIIter *itr, 

                            dvoid **elem, dvoid **elemind)

{

	return OCIIterGetCurrent(env, err, itr, elem, elemind);

}



sword SDO_OCIIterNext(OCIEnv *env, OCIError *err, OCIIter *itr, 

                      dvoid **elem, dvoid **elemind, boolean *eoc)

{

	return OCIIterNext(env, err, itr, elem, elemind, eoc);

}



sword SDO_OCIIterPrev(OCIEnv *env, OCIError *err, OCIIter *itr, 

                      dvoid **elem, dvoid **elemind, boolean *boc)

{

	return OCIIterPrev(env, err, itr, elem, elemind, boc);

}



sword SDO_OCITableSize(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

					   sb4 *size)

{

	return OCITableSize(env, err, tbl, size);

}



sword SDO_OCITableExists(OCIEnv *env, OCIError *err, CONST OCITable *tbl,

						 sb4 index, boolean *exists)

{

	return OCITableExists(env, err, tbl, index, exists);

}



sword SDO_OCITableDelete(OCIEnv *env, OCIError *err, sb4 index, OCITable *tbl)

{

	return OCITableDelete(env, err, index, tbl);

}



sword SDO_OCITableFirst(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

                        sb4 *index)

{

	return OCITableFirst(env, err, tbl, index);

}



sword SDO_OCITableLast(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

                       sb4 *index)

{

	return OCITableLast(env, err, tbl, index);

}



sword SDO_OCITableNext(OCIEnv *env, OCIError *err, sb4 index, 

					   CONST OCITable *tbl, sb4 *next_index, boolean *exists)

{

	return OCITableNext(env, err, index, tbl, next_index, exists);

}



sword SDO_OCITablePrev(OCIEnv *env, OCIError *err, sb4 index, 

                       CONST OCITable *tbl, sb4 *prev_index, boolean *exists)

{

	return OCITablePrev(env, err, index, tbl, prev_index, exists);

}





/********************************************************************************/

/* Part IV -- OCI open type manager interfaces                                  */

/********************************************************************************/

sword SDO_OCITypeIterNew(OCIEnv *env, OCIError *err, OCIType *tdo, 

                         OCITypeIter **iteratorOCI)

{

	return OCITypeIterNew(env, err, tdo, iteratorOCI);

}



sword SDO_OCITypeIterSet(OCIEnv *env, OCIError *err, OCIType *tdo, 

						 OCITypeIter *iteratorOCI)

{

	return OCITypeIterSet(env, err, tdo, iteratorOCI);

}



sword SDO_OCITypeIterFree(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI)

{

	return OCITypeIterFree(env, err, iteratorOCI);

}



sword SDO_OCITypeByName(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        CONST text *schema_name, ub4 s_length,

                        CONST text *type_name, ub4 t_length,

                        CONST text *version_name, ub4 v_length,

                        OCIDuration pin_duration, OCITypeGetOpt get_option,

                        OCIType **tdo)

{

	return OCITypeByName(env, err, svc, schema_name, s_length, type_name, 

						 t_length, version_name, v_length, pin_duration, 

						 get_option, tdo);

}



sword SDO_OCITypeArrayByName(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

                             ub4 array_len, CONST text *schema_name[], 

							 ub4 s_length[], CONST text *type_name[], 

							 ub4 t_length[], CONST text *version_name[], 

							 ub4 v_length[], OCIDuration pin_duration,

                             OCITypeGetOpt get_option, OCIType *tdo[])

{

	return OCITypeArrayByName(env, err, svc, array_len, schema_name, s_length, 

							  type_name, t_length, version_name, v_length, 

							  pin_duration, get_option, tdo);

}



sword SDO_OCITypeByRef(OCIEnv *env, OCIError *err, CONST OCIRef *type_ref, 

					   OCIDuration pin_duration,OCITypeGetOpt get_option, 

					   OCIType **tdo)

{

	return OCITypeByRef(env, err, type_ref, pin_duration, get_option, tdo);

}



sword SDO_OCITypeArrayByRef(OCIEnv *env, OCIError *err, ub4 array_len, 

							CONST OCIRef *type_ref[], OCIDuration pin_duration,

							OCITypeGetOpt get_option, OCIType *tdo[])

{

	return OCITypeArrayByRef(env, err, array_len, type_ref, pin_duration,

							 get_option, tdo);

}



text* SDO_OCITypeName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                      ub4 *n_length)

{

	return OCITypeName(env, err, tdo, n_length);

}



text* SDO_OCITypeSchema(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

						ub4 *n_length)

{

	return OCITypeSchema(env, err, tdo, n_length);

}



OCITypeCode SDO_OCITypeTypeCode(OCIEnv *env, OCIError *err, CONST OCIType *tdo)

{

	return OCITypeTypeCode(env, err, tdo);

}



OCITypeCode SDO_OCITypeCollTypeCode(OCIEnv *env, OCIError *err, 

									CONST OCIType *tdo)

{

	return OCITypeCollTypeCode(env, err, tdo);

}



text* SDO_OCITypeVersion(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                         ub4 *v_length)

{

	return OCITypeVersion(env, err, tdo, v_length);

}



ub4 SDO_OCITypeAttrs(OCIEnv *env, OCIError *err, CONST OCIType *tdo)

{

	return OCITypeAttrs(env, err, tdo);

}



ub4 SDO_OCITypeMethods(OCIEnv *env, OCIError *err, CONST OCIType *tdo)

{

	return OCITypeMethods(env, err, tdo);

}



text* SDO_OCITypeElemName(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem, 

                          ub4 *n_length)

{

	return OCITypeElemName(env, err, elem, n_length);

}



OCITypeCode SDO_OCITypeElemTypeCode(OCIEnv *env, OCIError *err,

                                    CONST OCITypeElem *elem)

{

	return OCITypeElemTypeCode(env, err, elem);

}



sword SDO_OCITypeElemType(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem, 

                          OCIType **elem_tdo)

{

	return OCITypeElemType(env, err, elem, elem_tdo);

}



ub4 SDO_OCITypeElemFlags(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemFlags(env, err, elem);

}



ub1 SDO_OCITypeElemNumPrec(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemNumPrec(env, err, elem);

}



sb1 SDO_OCITypeElemNumScale(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemNumScale(env, err, elem);

}



ub4 SDO_OCITypeElemLength(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemLength(env, err, elem);

}



ub2 SDO_OCITypeElemCharSetID(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemCharSetID(env, err, elem);

}



ub2 SDO_OCITypeElemCharSetForm(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem)

{

	return OCITypeElemCharSetForm(env, err, elem);

}



sword SDO_OCITypeElemParameterizedType(OCIEnv *env, OCIError *err,

                                       CONST OCITypeElem *elem, 

									   OCIType **type_stored)

{

	return OCITypeElemParameterizedType(env, err, elem, type_stored);

}



OCITypeCode SDO_OCITypeElemExtTypeCode(OCIEnv *env, OCIError *err,

                                       CONST OCITypeElem *elem)

{

	return OCITypeElemExtTypeCode(env, err, elem);

}



sword SDO_OCITypeAttrByName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                            CONST text *name, ub4 n_length, OCITypeElem **elem)

{

	return OCITypeAttrByName(env, err, tdo, name, n_length, elem);

}



sword SDO_OCITypeAttrNext(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI,

                          OCITypeElem **elem)

{

	return OCITypeAttrNext(env, err, iteratorOCI, elem);

}



sword SDO_OCITypeCollElem(OCIEnv *env, OCIError *err, CONST OCIType *tdo,

                          OCITypeElem **element)

{

	return OCITypeCollElem(env, err, tdo, element);

}



sword SDO_OCITypeCollSize(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                          ub4 *num_elems)

{

	return OCITypeCollSize(env, err, tdo, num_elems);

}



sword SDO_OCITypeCollExtTypeCode(OCIEnv *env, OCIError *err,

                                 CONST OCIType *tdo, OCITypeCode *sqt_code)

{

	return OCITypeCollExtTypeCode(env, err, tdo, sqt_code);

}



ub4 SDO_OCITypeMethodOverload(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                              CONST text *method_name, ub4 m_length)

{

	return OCITypeMethodOverload(env, err, tdo, method_name, m_length);

}



sword SDO_OCITypeMethodByName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                              CONST text *method_name, ub4 m_length,

                              OCITypeMethod **mdos)

{

	return OCITypeMethodByName(env, err, tdo, method_name, m_length, mdos);

}



sword SDO_OCITypeMethodNext(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI,

                            OCITypeMethod **mdo)

{

	return OCITypeMethodNext(env, err, iteratorOCI, mdo);

}



text *SDO_OCITypeMethodName(OCIEnv *env, OCIError *err,

                            CONST OCITypeMethod *mdo, ub4 *n_length)

{

	return OCITypeMethodName(env, err, mdo, n_length);

}



OCITypeEncap SDO_OCITypeMethodEncap(OCIEnv *env, OCIError *err,

                                    CONST OCITypeMethod *mdo)

{

	return OCITypeMethodEncap(env, err, mdo);

}



OCITypeMethodFlag SDO_OCITypeMethodFlags(OCIEnv *env, OCIError *err,

                                         CONST OCITypeMethod *mdo)

{

	return OCITypeMethodFlags(env, err, mdo);

}



sword SDO_OCITypeMethodMap(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                           OCITypeMethod **mdo)

{

	return OCITypeMethodMap(env, err, tdo, mdo);

}



sword SDO_OCITypeMethodOrder(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                             OCITypeMethod **mdo)

{

	return OCITypeMethodOrder(env, err, tdo, mdo);

}



ub4 SDO_OCITypeMethodParams(OCIEnv *env, OCIError *err,

                            CONST OCITypeMethod *mdo)

{

	return OCITypeMethodParams(env, err, mdo);

}



sword SDO_OCITypeResult(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo,

                        OCITypeElem **elem)

{

	return OCITypeResult(env, err, mdo, elem);

}



sword SDO_OCITypeParamByPos(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

							ub4 position, OCITypeElem **elem)

{

	return OCITypeParamByPos(env, err, mdo, position, elem);

}



sword SDO_OCITypeParamByName(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

                             CONST text *name, ub4 n_length, OCITypeElem **elem)

{

	return OCITypeParamByName(env, err, mdo, name, n_length, elem);

}



sword SDO_OCITypeParamPos(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

                          CONST text *name, ub4 n_length, ub4 *position, 

                          OCITypeElem **elem)

{

	return OCITypeParamPos(env, err, mdo, name, n_length, position, elem);

}



OCITypeParamMode SDO_OCITypeElemParamMode(OCIEnv *env, OCIError *err,

                                          CONST OCITypeElem *elem)

{

	return OCITypeElemParamMode(env, err, elem);

}



text* SDO_OCITypeElemDefaultValue(OCIEnv *env, OCIError *err,

                                  CONST OCITypeElem *elem, ub4 *d_v_length)

{

	return OCITypeElemDefaultValue(env, err, elem, d_v_length);

}



sword SDO_OCITypeVTInit(OCIEnv *env, OCIError *err)

{

	return OCITypeVTInit(env, err);

}

 

sword SDO_OCITypeVTInsert(OCIEnv *env, OCIError *err, CONST text *schema_name,

						  ub4 s_n_length, CONST text *type_name, ub4 t_n_length, 

						  CONST text *user_version, ub4 u_v_length)

{

	return OCITypeVTInsert(env, err, schema_name, s_n_length, type_name, 

						   t_n_length, user_version, u_v_length);

}



sword SDO_OCITypeVTSelect(OCIEnv *env, OCIError *err, CONST text *schema_name,

						  ub4 s_n_length, CONST text *type_name, ub4 t_n_length, 

						  text **user_version, ub4 *u_v_length, ub2 *version)

{

	return OCITypeVTSelect(env, err, schema_name, s_n_length, type_name, 

						   t_n_length, user_version, u_v_length, version);

}





