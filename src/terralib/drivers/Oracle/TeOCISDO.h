
/*! \file TeOCISDO.h
    
	This file was created by Oracle Corporation and contains functions to 
	works with the Oracle Call Interface (OCI) Library.
	Copyright 1998, Oracle SDO.  
*/

#ifndef SDO_OCI_H

#define SDO_OCI_H

#include "TeOracleDefines.h"

/** @name Part I -- OCI fundamental interfaces
*/
//@{
TLORACLE_DLL sword SDO_OCIEnvCreate(OCIEnv **envhpp, ub4 mode, dvoid *ctxp, 
						dvoid *(*malocfp)(dvoid *ctxp, size_t size),
						dvoid *(*ralocfp)(dvoid *ctxp, dvoid *memptr, size_t newsize),
						void (*mfreefp)(dvoid *ctxp, dvoid *memptr),
						size_t xtramensz, dvoid **usrmempp); 

TLORACLE_DLL sword SDO_OCIInitialize(ub4 mode, dvoid *ctxp, 
						dvoid *(*malocfp)(dvoid *ctxp, size_t size),
						dvoid *(*ralocfp)(dvoid *ctxp, dvoid *memptr, size_t newsize),
						void (*mfreefp)(dvoid *ctxp, dvoid *memptr));

TLORACLE_DLL sword SDO_OCIHandleAlloc(CONST dvoid *parenth, dvoid **hndlpp, ub4 type, 
						 size_t xtramem_sz, dvoid **usrmempp);

TLORACLE_DLL sword SDO_OCIHandleFree(dvoid *hndlp, ub4 type);

TLORACLE_DLL sword SDO_OCIDescriptorAlloc(CONST dvoid *parenth, dvoid **descpp, ub4 type,
							 size_t xtramem_sz, dvoid **usrmempp);

TLORACLE_DLL sword SDO_OCIDescriptorFree(dvoid *descp, ub4 type);

TLORACLE_DLL sword SDO_OCIEnvInit(OCIEnv **envp, ub4 mode, size_t xtramem_sz, 
					 dvoid **usrmempp);

TLORACLE_DLL sword SDO_OCIServerAttach(OCIServer *srvhp, OCIError *errhp,
						  CONST text *dblink, sb4 dblink_len, ub4 mode);

TLORACLE_DLL sword SDO_OCIServerDetach(OCIServer *srvhp, OCIError *errhp, ub4 mode);

TLORACLE_DLL sword SDO_OCISessionBegin(OCISvcCtx *svchp, OCIError *errhp, 
						  OCISession *usrhp, ub4 credt, ub4 mode);

TLORACLE_DLL sword SDO_OCISessionEnd(OCISvcCtx *svchp, OCIError *errhp, 
						OCISession *usrhp, ub4 mode);

TLORACLE_DLL sword SDO_OCILogon(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp, 
				   CONST text *username, ub4 uname_len, CONST text *password, 
				   ub4 passwd_len, CONST text *dbname, ub4 dbname_len);

TLORACLE_DLL sword SDO_OCILogoff(OCISvcCtx *svchp, OCIError *errhp);

TLORACLE_DLL sword SDO_OCIPasswordChange(OCISvcCtx *svchp, OCIError *errhp, 
							CONST text *user_name, ub4 usernm_len, 
							CONST text *opasswd, ub4 opasswd_len, 
							CONST text *npasswd, ub4 npasswd_len, ub4 mode);

TLORACLE_DLL sword SDO_OCIStmtPrepare(OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp, CONST OraText *stmt,
						 ub4 stmt_len, CONST OraText *key, ub4 keylen, ub4 language, ub4 mode);

TLORACLE_DLL sword SDO_OCIBindByPos(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
					   ub4 position, dvoid *valuep, sb4 value_sz,
					   ub2 dty, dvoid *indp, ub2 *alenp, ub2 *rcodep,
					   ub4 maxarr_len, ub4 *curelep, ub4 mode);

TLORACLE_DLL sword SDO_OCIBindByName(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
						CONST text *placeholder, sb4 placeh_len, 
						dvoid *valuep, sb4 value_sz, ub2 dty, 
						dvoid *indp, ub2 *alenp, ub2 *rcodep, 
						ub4 maxarr_len, ub4 *curelep, ub4 mode);

TLORACLE_DLL sword SDO_OCIBindObject(OCIBind *bindp, OCIError *errhp, CONST OCIType *type, 
						dvoid **pgvpp, ub4 *pvszsp, dvoid **indpp, ub4 *indszp);

TLORACLE_DLL sword SDO_OCIBindDynamic(OCIBind *bindp, OCIError *errhp, dvoid *ictxp,
						 OCICallbackInBind icbfp, dvoid *octxp,
						 OCICallbackOutBind ocbfp);

TLORACLE_DLL sword SDO_OCIBindArrayOfStruct(OCIBind *bindp, OCIError *errhp, 
                               ub4 pvskip, ub4 indskip, ub4 alskip, ub4 rcskip);

TLORACLE_DLL sword SDO_OCIStmtGetPieceInfo(OCIStmt *stmtp, OCIError *errhp, 
                              dvoid **hndlpp, ub4 *typep, ub1 *in_outp, 
							  ub4 *iterp, ub4 *idxp, ub1 *piecep);

TLORACLE_DLL sword SDO_OCIStmtSetPieceInfo(dvoid *hndlp, ub4 type, OCIError *errhp, 
                              CONST dvoid *bufp, ub4 *alenp, ub1 piece, 
                              CONST dvoid *indp, ub2 *rcodep);

TLORACLE_DLL sword SDO_OCIStmtExecute(OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp, 
						 ub4 iters, ub4 rowoff, CONST OCISnapshot *snap_in, 
						 OCISnapshot *snap_out, ub4 mode);

TLORACLE_DLL sword SDO_OCIDefineByPos(OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,
						 ub4 position, dvoid *valuep, sb4 value_sz, ub2 dty,
						 dvoid *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode);

TLORACLE_DLL sword SDO_OCIDefineObject(OCIDefine *defnp, OCIError *errhp, 
                          CONST OCIType *type, dvoid **pgvpp, 
                          ub4 *pvszsp, dvoid **indpp, ub4 *indszp);

TLORACLE_DLL sword SDO_OCIDefineDynamic(OCIDefine *defnp, OCIError *errhp, dvoid *octxp,
                           OCICallbackDefine ocbfp);

TLORACLE_DLL sword SDO_OCIDefineArrayOfStruct(OCIDefine *defnp, OCIError *errhp, ub4 pvskip,
                                 ub4 indskip, ub4 rlskip, ub4 rcskip);

TLORACLE_DLL sword SDO_OCIStmtFetch(OCIStmt *stmtp, OCIError *errhp, ub4 nrows, 
                       ub2 orientation, ub4 mode);

TLORACLE_DLL sword SDO_OCIStmtGetBindInfo(OCIStmt *stmtp, OCIError *errhp, ub4 size, 
                             ub4 startloc, sb4 *found, text *bvnp[], ub1 bvnl[],
                             text *invp[], ub1 inpl[], ub1 dupl[], OCIBind *hndl[]);

TLORACLE_DLL sword SDO_OCIDescribeAny(OCISvcCtx *svchp, OCIError *errhp, dvoid *objptr, 
                         ub4 objnm_len, ub1 objptr_typ, ub1 info_level,
						 ub1 objtyp, OCIDescribe *dschp);

TLORACLE_DLL sword SDO_OCIParamGet(CONST dvoid *hndlp, ub4 htype, OCIError *errhp, 
                      dvoid **parmdpp, ub4 pos);

TLORACLE_DLL sword SDO_OCIParamSet(dvoid *hdlp, ub4 htyp, OCIError *errhp, CONST dvoid *dscp,
                      ub4 dtyp, ub4 pos);

TLORACLE_DLL sword SDO_OCITransStart(OCISvcCtx *svchp, OCIError *errhp, uword timeout, ub4 flags);

TLORACLE_DLL sword SDO_OCITransDetach(OCISvcCtx *svchp, OCIError *errhp, ub4 flags );

TLORACLE_DLL sword SDO_OCITransCommit(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);

TLORACLE_DLL sword SDO_OCITransRollback(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);

TLORACLE_DLL sword SDO_OCITransPrepare(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);

TLORACLE_DLL sword SDO_OCITransForget(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);

TLORACLE_DLL sword SDO_OCIErrorGet(dvoid *hndlp, ub4 recordno, text *sqlstate,
                      sb4 *errcodep, text *bufp, ub4 bufsiz, ub4 type);

TLORACLE_DLL sword SDO_OCILobAppend(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *dst_locp,
                       OCILobLocator *src_locp);

TLORACLE_DLL sword SDO_OCILobAssign(OCIEnv *envhp, OCIError *errhp, CONST OCILobLocator *src_locp, 
                       OCILobLocator **dst_locpp);

TLORACLE_DLL sword SDO_OCILobCharSetForm(OCIEnv *envhp, OCIError *errhp, 
							CONST OCILobLocator *locp, ub1 *csfrm);

TLORACLE_DLL sword SDO_OCILobCharSetId(OCIEnv *envhp, OCIError *errhp, 
						  CONST OCILobLocator *locp, ub2 *csid);

TLORACLE_DLL sword SDO_OCILobCopy(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *dst_locp,
                     OCILobLocator *src_locp, ub4 amount, ub4 dst_offset, 
                     ub4 src_offset);

TLORACLE_DLL sword SDO_OCILobDisableBuffering(OCISvcCtx *svchp, OCIError *errhp,
								 OCILobLocator *locp);

TLORACLE_DLL sword SDO_OCILobEnableBuffering(OCISvcCtx *svchp, OCIError *errhp,
								OCILobLocator *locp);

TLORACLE_DLL sword SDO_OCILobErase(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                      ub4 *amount, ub4 offset);

TLORACLE_DLL sword SDO_OCILobFileClose(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep);

TLORACLE_DLL sword SDO_OCILobFileCloseAll(OCISvcCtx *svchp, OCIError *errhp);

TLORACLE_DLL sword SDO_OCILobFileExists(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,
						   boolean *flag);

TLORACLE_DLL sword SDO_OCILobFileGetName(OCIEnv *envhp, OCIError *errhp, 
							CONST OCILobLocator *filep, text *dir_alias, 
							ub2 *d_length, text *filename, ub2 *f_length);

TLORACLE_DLL sword SDO_OCILobFileIsOpen(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,
                           boolean *flag);

TLORACLE_DLL sword SDO_OCILobFileOpen(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *filep,
                         ub1 mode);

TLORACLE_DLL sword SDO_OCILobFileSetName(OCIEnv *envhp, OCIError *errhp, OCILobLocator **filepp, 
                            CONST text *dir_alias, ub2 d_length, 
                            CONST text *filename, ub2 f_length);

TLORACLE_DLL sword SDO_OCILobFlushBuffer(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                            ub4 flag);

TLORACLE_DLL sword SDO_OCILobGetLength(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                          ub4 *lenp);

TLORACLE_DLL sword SDO_OCILobIsEqual(OCIEnv *envhp, CONST OCILobLocator *x, 
						CONST OCILobLocator *y, boolean *is_equal);

TLORACLE_DLL sword SDO_OCILobLoadFromFile(OCISvcCtx *svchp, OCIError *errhp, 
							 OCILobLocator *dst_locp, OCILobLocator *src_filep, 
                             ub4 amount, ub4 dst_offset, ub4 src_offset);

TLORACLE_DLL sword SDO_OCILobLocatorIsInit(OCIEnv *envhp, OCIError *errhp, 
							  CONST OCILobLocator *locp, boolean *is_initialized);

TLORACLE_DLL sword SDO_OCILobRead(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                     ub4 *amtp, ub4 offset, dvoid *bufp, ub4 bufl, 
                     dvoid *ctxp, sb4 (*cbfp)(dvoid *ctxp, CONST dvoid *bufp, 
					 ub4 len, ub1 piece), ub2 csid, ub1 csfrm);

TLORACLE_DLL sword SDO_OCILobTrim(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                     ub4 newlen);

TLORACLE_DLL sword SDO_OCILobWrite(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                      ub4 *amtp, ub4 offset, dvoid *bufp, ub4 buflen, ub1 piece, 
					  dvoid *ctxp, sb4 (*cbfp)(dvoid *ctxp, dvoid *bufp, ub4 *len, 
					  ub1 *piece), ub2 csid, ub1 csfrm);

TLORACLE_DLL sword SDO_OCIBreak(dvoid *hndlp, OCIError *errhp);


TLORACLE_DLL sword SDO_OCIServerVersion(dvoid *hndlp, OCIError *errhp, text *bufp, ub4 bufsz,
                           ub1 hndltype);

TLORACLE_DLL sword SDO_OCIAttrGet(CONST dvoid *trgthndlp, ub4 trghndltyp, dvoid *attributep, 
					 ub4 *sizep, ub4 attrtype, OCIError *errhp);


TLORACLE_DLL sword SDO_OCIAttrSet(dvoid *trgthndlp, ub4 trghndltyp, dvoid *attributep,
					 ub4 size, ub4 attrtype, OCIError *errhp);

TLORACLE_DLL sword SDO_OCISvcCtxToLda(OCISvcCtx *svchp, OCIError *errhp, Lda_Def *ldap);

TLORACLE_DLL sword SDO_OCILdaToSvcCtx(OCISvcCtx **svchpp, OCIError *errhp, Lda_Def *ldap);

TLORACLE_DLL sword SDO_OCIResultSetToStmt(OCIResult *rsetdp, OCIError *errhp);
//@}  


/** @name Part II -- OCI navigational interfaces
*/
//@{
TLORACLE_DLL sword SDO_OCIObjectNew(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

					   OCITypeCode typecode, OCIType *tdo, dvoid *table, 

					   OCIDuration duration, boolean value, dvoid **instance);



TLORACLE_DLL sword SDO_OCIObjectPin(OCIEnv *env, OCIError *err, OCIRef *object_ref, 

					   OCIComplexObject *corhdl, OCIPinOpt pin_option, 

					   OCIDuration pin_duration, OCILockOpt lock_option, 

					   dvoid **object);



TLORACLE_DLL sword SDO_OCIObjectUnpin(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectPinCountReset(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectLock(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectMarkUpdate(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectUnmark(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectUnmarkByRef(OCIEnv *env, OCIError *err, OCIRef *ref);



TLORACLE_DLL sword SDO_OCIObjectFree(OCIEnv *env, OCIError *err, dvoid *instance, 

                        ub2 flags);



TLORACLE_DLL sword SDO_OCIObjectMarkDeleteByRef(OCIEnv *env, OCIError *err, OCIRef *object_ref);



TLORACLE_DLL sword SDO_OCIObjectMarkDelete(OCIEnv *env, OCIError *err, dvoid *instance);



TLORACLE_DLL sword SDO_OCIObjectFlush(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectRefresh(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectCopy(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        dvoid *source, dvoid *null_source, dvoid *target, 

						dvoid *null_target, OCIType *tdo, OCIDuration duration, 

						ub1 option);



TLORACLE_DLL sword SDO_OCIObjectGetTypeRef(OCIEnv *env, OCIError *err, dvoid *instance, 

                              OCIRef *type_ref);



TLORACLE_DLL sword SDO_OCIObjectGetObjectRef(OCIEnv *env, OCIError *err, dvoid *object, 

                                OCIRef *object_ref);



TLORACLE_DLL sword SDO_OCIObjectGetInd(OCIEnv *env, OCIError *err, dvoid *instance, 

                          dvoid **null_struct);



TLORACLE_DLL sword SDO_OCIObjectExists(OCIEnv *env, OCIError *err, dvoid *ins, boolean *exist);



TLORACLE_DLL sword SDO_OCIObjectGetProperty(OCIEnv *envh, OCIError *errh, CONST dvoid *obj, 

							   OCIObjectPropId propertyId, dvoid *property, 

							   ub4 *size);



TLORACLE_DLL sword SDO_OCIObjectIsLocked(OCIEnv *env, OCIError *err, dvoid *ins, boolean *lock);



TLORACLE_DLL sword SDO_OCIObjectIsDirty(OCIEnv *env, OCIError *err, dvoid *ins, boolean *dirty);



TLORACLE_DLL sword SDO_OCIObjectPinTable(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

							CONST text *schema_name, ub4 s_n_length, 

							CONST text *object_name, ub4 o_n_length, 

							CONST OCIRef *scope_obj_ref, OCIDuration pin_duration, 

							dvoid** object);



TLORACLE_DLL sword SDO_OCIObjectArrayPin(OCIEnv *env, OCIError *err, OCIRef **ref_array, 

							ub4 array_size, OCIComplexObject **cor_array,

							ub4 cor_array_size, OCIPinOpt pin_option, 

							OCIDuration pin_duration, OCILockOpt lock, 

							dvoid **obj_array, ub4 *pos);



TLORACLE_DLL sword SDO_OCICacheFlush(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

						dvoid *context, OCIRef *(*get)(dvoid *context, ub1 *last),

						OCIRef **ref);



TLORACLE_DLL sword SDO_OCICacheRefresh(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

						  OCIRefreshOpt option, dvoid *context,

						  OCIRef *(*get)(dvoid *context), OCIRef **ref);



TLORACLE_DLL sword SDO_OCICacheUnpin(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc);



TLORACLE_DLL sword SDO_OCICacheFree(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc); 



TLORACLE_DLL sword SDO_OCICacheUnmark(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc);



TLORACLE_DLL sword SDO_OCIDurationBegin(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                           OCIDuration parent, OCIDuration *dur);



TLORACLE_DLL sword SDO_OCIDurationEnd(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                         OCIDuration duration);



TLORACLE_DLL sword SDO_OCIDurationGetParent(OCIEnv *env, OCIError *err, 

                               OCIDuration duration, OCIDuration *parent);



TLORACLE_DLL sword SDO_OCIObjectAlwaysLatest(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectNotAlwaysLatest(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectFlushRefresh(OCIEnv *env, OCIError *err, dvoid *object);



TLORACLE_DLL sword SDO_OCIObjectIsLoaded(OCIEnv *env, OCIError *err, dvoid *ins, 

							boolean *load);



TLORACLE_DLL sword SDO_OCIObjectIsDirtied(OCIEnv *env, OCIError *err, dvoid *ins,

							 boolean *dirty);



TLORACLE_DLL sword SDO_OCICacheGetObjects(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

							 OCIObjectProperty property, dvoid *client_context,

							 void (*client_callback)(dvoid *client_context,

							 dvoid *object));



TLORACLE_DLL sword SDO_OCICacheRegister(OCIEnv *env, OCIError *err, OCIObjectEvent event,

						   dvoid *client_context, void (*client_callback)(

						   dvoid *client_context, OCIObjectEvent event,

						   dvoid *object));



TLORACLE_DLL sword SDO_OCICacheFlushRefresh(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

							   dvoid *context, OCIRef *(*get)(dvoid *context, 

							   ub1 *last), OCIRef **ref);

//@}


/** @name Part III -- OCI primitive OTS interfaces  
*/
//@{
TLORACLE_DLL void SDO_OCINumberSetZero(OCIError *err, OCINumber *num);



TLORACLE_DLL sword SDO_OCINumberAdd(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberSub(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberMul(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberDiv(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberMod(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberIntPower(OCIError *err, CONST OCINumber *base,

                            CONST sword exp, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberNeg(OCIError *err, CONST OCINumber *number,

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberToText(OCIError *err, CONST OCINumber *number, 

                          CONST text *fmt, ub4 fmt_length,

                          CONST text *nls_params, ub4 nls_p_length,

                          ub4 *buf_size, text *buf);



TLORACLE_DLL sword SDO_OCINumberFromText(OCIError *err, CONST text *str, ub4 str_length, 

							CONST text *fmt, ub4 fmt_length, 

							CONST text *nls_params, ub4 nls_p_length, 

							OCINumber *number);



TLORACLE_DLL sword SDO_OCINumberToInt(OCIError *err, CONST OCINumber *number,

                         uword rsl_length, uword rsl_flag, dvoid *rsl);



TLORACLE_DLL sword SDO_OCINumberFromInt(OCIError *err, CONST dvoid *inum, uword inum_length,

						   uword inum_s_flag, OCINumber *number);



TLORACLE_DLL sword SDO_OCINumberToReal(OCIError *err, CONST OCINumber *number,

                          uword rsl_length, dvoid *rsl);



TLORACLE_DLL sword SDO_OCINumberFromReal(OCIError *err, CONST dvoid *rnum,

                            uword rnum_length, OCINumber *number);



TLORACLE_DLL sword SDO_OCINumberCmp(OCIError *err, CONST OCINumber *number1, 

                       CONST OCINumber *number2, sword *result);



TLORACLE_DLL sword SDO_OCINumberSign(OCIError *err, CONST OCINumber *number,

                        sword *result);



TLORACLE_DLL sword SDO_OCINumberIsZero(OCIError *err, CONST OCINumber *number,

                          boolean *result);



TLORACLE_DLL sword SDO_OCINumberAssign(OCIError *err, CONST OCINumber *from,

                          OCINumber *to);



TLORACLE_DLL sword SDO_OCINumberAbs(OCIError *err, CONST OCINumber *number,

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberCeil(OCIError *err, CONST OCINumber *number,

                        OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberFloor(OCIError *err, CONST OCINumber *number, 

                         OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberSqrt(OCIError *err, CONST OCINumber *number, 

                        OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberTrunc(OCIError *err, CONST OCINumber *number, 

                         sword decplace, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberPower(OCIError *err, CONST OCINumber *base, 

                         CONST OCINumber *number, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberRound(OCIError *err, CONST OCINumber *number,

                         sword decplace, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberSin(OCIError *err, CONST OCINumber *number,

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberArcSin(OCIError *err, CONST OCINumber *number,

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberHypSin(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberCos(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberArcCos(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberHypCos(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberTan(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberArcTan(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberArcTan2(OCIError *err, CONST OCINumber *number1, 

                           CONST OCINumber *number2, OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberHypTan(OCIError *err, CONST OCINumber *number, 

                          OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberExp(OCIError *err, CONST OCINumber *number, 

                       OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberLn(OCIError *err, CONST OCINumber *number, 

                      OCINumber *result);



TLORACLE_DLL sword SDO_OCINumberLog(OCIError *err, CONST OCINumber *base, 

                       CONST OCINumber *number, OCINumber *result);



TLORACLE_DLL sword SDO_OCIDateAssign(OCIError *err, CONST OCIDate *from, OCIDate *to);



TLORACLE_DLL sword SDO_OCIDateToText(OCIError *err, CONST OCIDate *date, 

                        CONST text *fmt, ub1 fmt_length, 

                        CONST text *lang_name, ub4 lang_length, 

                        ub4 *buf_size, text *buf);



TLORACLE_DLL sword SDO_OCIDateFromText(OCIError *err, CONST text *date_str, 

						  ub4 d_str_length, CONST text *fmt, ub1 fmt_length, 

						  CONST text *lang_name, ub4 lang_length, 

						  OCIDate *date);



TLORACLE_DLL sword SDO_OCIDateCompare(OCIError *err, CONST OCIDate *date1, 

						 CONST OCIDate *date2, sword *result);



TLORACLE_DLL sword SDO_OCIDateAddMonths(OCIError *err, CONST OCIDate *date, sb4 num_months,

                           OCIDate *result);



TLORACLE_DLL sword SDO_OCIDateAddDays(OCIError *err, CONST OCIDate *date, sb4 num_days,

                         OCIDate *result);



TLORACLE_DLL sword SDO_OCIDateLastDay(OCIError *err, CONST OCIDate *date, 

                         OCIDate *last_day);



TLORACLE_DLL sword SDO_OCIDateDaysBetween(OCIError *err, CONST OCIDate *date1, 

                             CONST OCIDate *date2, sb4 *num_days);



TLORACLE_DLL sword SDO_OCIDateZoneToZone(OCIError *err, CONST OCIDate *date1,

                            CONST text *zon1, ub4 zon1_length, 

							CONST text *zon2, ub4 zon2_length, OCIDate *date2);



TLORACLE_DLL sword SDO_OCIDateNextDay(OCIError *err, CONST OCIDate *date, CONST text *day_p, 

                         ub4 day_length, OCIDate *next_day);





TLORACLE_DLL sword SDO_OCIDateCheck(OCIError *err, CONST OCIDate *date, uword *valid);



TLORACLE_DLL sword SDO_OCIDateSysDate(OCIError *err, OCIDate *sys_date);



TLORACLE_DLL sword SDO_OCIStringAssign(OCIEnv *env, OCIError *err, CONST OCIString *rhs, 

                          OCIString **lhs);



TLORACLE_DLL sword SDO_OCIStringAssignText(OCIEnv *env, OCIError *err, CONST text *rhs, 

                              ub4 rhs_len, OCIString **lhs);



TLORACLE_DLL sword SDO_OCIStringResize(OCIEnv *env, OCIError *err, ub4 new_size, 

                          OCIString **str);



TLORACLE_DLL ub4 SDO_OCIStringSize(OCIEnv *env, CONST OCIString *vs);



TLORACLE_DLL text *SDO_OCIStringPtr(OCIEnv *env, CONST OCIString *vs);



TLORACLE_DLL sword SDO_OCIStringAllocSize(OCIEnv *env, OCIError *err, CONST OCIString *vs, 

                             ub4 *allocsize);



TLORACLE_DLL sword SDO_OCIRawAssignRaw(OCIEnv *env, OCIError *err, CONST OCIRaw *rhs, 

                          OCIRaw **lhs);



TLORACLE_DLL sword SDO_OCIRawAssignBytes(OCIEnv *env, OCIError *err, CONST ub1 *rhs, 

                            ub4 rhs_len, OCIRaw **lhs);



TLORACLE_DLL sword SDO_OCIRawResize(OCIEnv *env, OCIError *err, ub4 new_size, OCIRaw **raw);



TLORACLE_DLL ub4 SDO_OCIRawSize(OCIEnv *env, CONST OCIRaw *raw);



TLORACLE_DLL ub1 *SDO_OCIRawPtr(OCIEnv *env, CONST OCIRaw *raw);



TLORACLE_DLL sword SDO_OCIRawAllocSize(OCIEnv *env, OCIError *err, CONST OCIRaw *raw,

						  ub4 *allocsize);



TLORACLE_DLL void SDO_OCIRefClear(OCIEnv *env, OCIRef *ref);



TLORACLE_DLL sword SDO_OCIRefAssign(OCIEnv *env, OCIError *err, CONST OCIRef *source, 

                       OCIRef **target);



TLORACLE_DLL boolean SDO_OCIRefIsEqual(OCIEnv *env, CONST OCIRef *x, CONST OCIRef *y);



TLORACLE_DLL boolean SDO_OCIRefIsNull(OCIEnv *env, CONST OCIRef *ref);



TLORACLE_DLL ub4 SDO_OCIRefHexSize(OCIEnv *env, CONST OCIRef *ref);



TLORACLE_DLL sword SDO_OCIRefFromHex(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        CONST text *hex, ub4 length, OCIRef **ref);



TLORACLE_DLL sword SDO_OCIRefToHex(OCIEnv *env, OCIError *err, CONST OCIRef *ref, 

                      text *hex, ub4 *hex_length);



TLORACLE_DLL sword SDO_OCICollSize(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

					  sb4 *size);



TLORACLE_DLL sb4 SDO_OCICollMax(OCIEnv *env, CONST OCIColl *coll);



TLORACLE_DLL sword SDO_OCICollGetElem(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                         sb4 index, boolean *exists, dvoid **elem, 

                         dvoid **elemind);



TLORACLE_DLL sword SDO_OCICollAssignElem(OCIEnv *env, OCIError *err, sb4 index, 

                            CONST dvoid *elem, 

                            CONST dvoid *elemind, OCIColl *coll);



TLORACLE_DLL sword SDO_OCICollAssign(OCIEnv *env, OCIError *err, CONST OCIColl *rhs, 

                        OCIColl *lhs);



TLORACLE_DLL sword SDO_OCICollAppend(OCIEnv *env, OCIError *err, CONST dvoid *elem, 

                        CONST dvoid *elemind, OCIColl *coll);



TLORACLE_DLL sword SDO_OCICollTrim(OCIEnv *env, OCIError *err, sb4 trim_num, 

                      OCIColl *coll);



TLORACLE_DLL sword SDO_OCIIterCreate(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                        OCIIter **itr);



TLORACLE_DLL sword SDO_OCIIterDelete(OCIEnv *env, OCIError *err, OCIIter **itr);



TLORACLE_DLL sword SDO_OCIIterInit(OCIEnv *env, OCIError *err, CONST OCIColl *coll, 

                      OCIIter *itr);



TLORACLE_DLL sword SDO_OCIIterGetCurrent(OCIEnv *env, OCIError *err, CONST OCIIter *itr, 

                            dvoid **elem, dvoid **elemind);



TLORACLE_DLL sword SDO_OCIIterNext(OCIEnv *env, OCIError *err, OCIIter *itr, 

                      dvoid **elem, dvoid **elemind, boolean *eoc);



TLORACLE_DLL sword SDO_OCIIterPrev(OCIEnv *env, OCIError *err, OCIIter *itr, 

                      dvoid **elem, dvoid **elemind, boolean *boc);



TLORACLE_DLL sword SDO_OCITableSize(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

					   sb4 *size);



TLORACLE_DLL sword SDO_OCITableExists(OCIEnv *env, OCIError *err, CONST OCITable *tbl,

						 sb4 index, boolean *exists);



TLORACLE_DLL sword SDO_OCITableDelete(OCIEnv *env, OCIError *err, sb4 index, OCITable *tbl);



TLORACLE_DLL sword SDO_OCITableFirst(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

                        sb4 *index);



TLORACLE_DLL sword SDO_OCITableLast(OCIEnv *env, OCIError *err, CONST OCITable *tbl, 

                       sb4 *index);



TLORACLE_DLL sword SDO_OCITableNext(OCIEnv *env, OCIError *err, sb4 index, 

					   CONST OCITable *tbl, sb4 *next_index, boolean *exists);



TLORACLE_DLL sword SDO_OCITablePrev(OCIEnv *env, OCIError *err, sb4 index, 

                       CONST OCITable *tbl, sb4 *prev_index, boolean *exists);
//@}

/** @name Part IV -- OCI open type manager interfaces
*/
//@{
TLORACLE_DLL sword SDO_OCITypeIterNew(OCIEnv *env, OCIError *err, OCIType *tdo, 

                         OCITypeIter **iteratorOCI);



TLORACLE_DLL sword SDO_OCITypeIterSet(OCIEnv *env, OCIError *err, OCIType *tdo, 

						 OCITypeIter *iteratorOCI);



TLORACLE_DLL sword SDO_OCITypeIterFree(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI);



TLORACLE_DLL sword SDO_OCITypeByName(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc, 

                        CONST text *schema_name, ub4 s_length,

                        CONST text *type_name, ub4 t_length,

                        CONST text *version_name, ub4 v_length,

                        OCIDuration pin_duration, OCITypeGetOpt get_option,

                        OCIType **tdo);



TLORACLE_DLL sword SDO_OCITypeArrayByName(OCIEnv *env, OCIError *err, CONST OCISvcCtx *svc,

                             ub4 array_len, CONST text *schema_name[], 

							 ub4 s_length[], CONST text *type_name[], 

							 ub4 t_length[], CONST text *version_name[], 

							 ub4 v_length[], OCIDuration pin_duration,

                             OCITypeGetOpt get_option, OCIType *tdo[]);



TLORACLE_DLL sword SDO_OCITypeByRef(OCIEnv *env, OCIError *err, CONST OCIRef *type_ref, 

					   OCIDuration pin_duration,OCITypeGetOpt get_option, 

					   OCIType **tdo);



TLORACLE_DLL sword SDO_OCITypeArrayByRef(OCIEnv *env, OCIError *err, ub4 array_len, 

							CONST OCIRef *type_ref[], OCIDuration pin_duration,

							OCITypeGetOpt get_option, OCIType *tdo[]);



TLORACLE_DLL text* SDO_OCITypeName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                      ub4 *n_length);



TLORACLE_DLL text* SDO_OCITypeSchema(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

						ub4 *n_length);



TLORACLE_DLL OCITypeCode SDO_OCITypeTypeCode(OCIEnv *env, OCIError *err, CONST OCIType *tdo);



TLORACLE_DLL OCITypeCode SDO_OCITypeCollTypeCode(OCIEnv *env, OCIError *err, 

									CONST OCIType *tdo);



TLORACLE_DLL text* SDO_OCITypeVersion(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                         ub4 *v_length);



TLORACLE_DLL ub4 SDO_OCITypeAttrs(OCIEnv *env, OCIError *err, CONST OCIType *tdo);



TLORACLE_DLL ub4 SDO_OCITypeMethods(OCIEnv *env, OCIError *err, CONST OCIType *tdo);



TLORACLE_DLL text* SDO_OCITypeElemName(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem, 

                          ub4 *n_length);



TLORACLE_DLL OCITypeCode SDO_OCITypeElemTypeCode(OCIEnv *env, OCIError *err,

                                    CONST OCITypeElem *elem);



TLORACLE_DLL sword SDO_OCITypeElemType(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem, 

                          OCIType **elem_tdo);



TLORACLE_DLL ub4 SDO_OCITypeElemFlags(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL ub1 SDO_OCITypeElemNumPrec(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL sb1 SDO_OCITypeElemNumScale(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL ub4 SDO_OCITypeElemLength(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL ub2 SDO_OCITypeElemCharSetID(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL ub2 SDO_OCITypeElemCharSetForm(OCIEnv *env, OCIError *err, CONST OCITypeElem *elem);



TLORACLE_DLL sword SDO_OCITypeElemParameterizedType(OCIEnv *env, OCIError *err,

                                       CONST OCITypeElem *elem, 

									   OCIType **type_stored);



TLORACLE_DLL OCITypeCode SDO_OCITypeElemExtTypeCode(OCIEnv *env, OCIError *err,

                                       CONST OCITypeElem *elem);



TLORACLE_DLL sword SDO_OCITypeAttrByName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                            CONST text *name, ub4 n_length, OCITypeElem **elem);



TLORACLE_DLL sword SDO_OCITypeAttrNext(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI,

                          OCITypeElem **elem);



TLORACLE_DLL sword SDO_OCITypeCollElem(OCIEnv *env, OCIError *err, CONST OCIType *tdo,

                          OCITypeElem **element);



TLORACLE_DLL sword SDO_OCITypeCollSize(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                          ub4 *num_elems);



TLORACLE_DLL sword SDO_OCITypeCollExtTypeCode(OCIEnv *env, OCIError *err,

                                 CONST OCIType *tdo, OCITypeCode *sqt_code);



TLORACLE_DLL ub4 SDO_OCITypeMethodOverload(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                              CONST text *method_name, ub4 m_length);



TLORACLE_DLL sword SDO_OCITypeMethodByName(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                              CONST text *method_name, ub4 m_length,

                              OCITypeMethod **mdos);



TLORACLE_DLL sword SDO_OCITypeMethodNext(OCIEnv *env, OCIError *err, OCITypeIter *iteratorOCI,

                            OCITypeMethod **mdo);



TLORACLE_DLL text *SDO_OCITypeMethodName(OCIEnv *env, OCIError *err,

                            CONST OCITypeMethod *mdo, ub4 *n_length);



TLORACLE_DLL OCITypeEncap SDO_OCITypeMethodEncap(OCIEnv *env, OCIError *err,

                                    CONST OCITypeMethod *mdo);



TLORACLE_DLL OCITypeMethodFlag SDO_OCITypeMethodFlags(OCIEnv *env, OCIError *err,

                                         CONST OCITypeMethod *mdo);



TLORACLE_DLL sword SDO_OCITypeMethodMap(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                           OCITypeMethod **mdo);



TLORACLE_DLL sword SDO_OCITypeMethodOrder(OCIEnv *env, OCIError *err, CONST OCIType *tdo, 

                             OCITypeMethod **mdo);



TLORACLE_DLL ub4 SDO_OCITypeMethodParams(OCIEnv *env, OCIError *err,

                            CONST OCITypeMethod *mdo);



TLORACLE_DLL sword SDO_OCITypeResult(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo,

                        OCITypeElem **elem);



TLORACLE_DLL sword SDO_OCITypeParamByPos(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

							ub4 position, OCITypeElem **elem);



TLORACLE_DLL sword SDO_OCITypeParamByName(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

                             CONST text *name, ub4 n_length, OCITypeElem **elem);



TLORACLE_DLL sword SDO_OCITypeParamPos(OCIEnv *env, OCIError *err, CONST OCITypeMethod *mdo, 

                          CONST text *name, ub4 n_length, ub4 *position, 

                          OCITypeElem **elem);



TLORACLE_DLL OCITypeParamMode SDO_OCITypeElemParamMode(OCIEnv *env, OCIError *err,

                                          CONST OCITypeElem *elem);



TLORACLE_DLL text* SDO_OCITypeElemDefaultValue(OCIEnv *env, OCIError *err,

                                  CONST OCITypeElem *elem, ub4 *d_v_length);



TLORACLE_DLL sword SDO_OCITypeVTInit(OCIEnv *env, OCIError *err);

 

TLORACLE_DLL sword SDO_OCITypeVTInsert(OCIEnv *env, OCIError *err, CONST text *schema_name,

						  ub4 s_n_length, CONST text *type_name, ub4 t_n_length, 

						  CONST text *user_version, ub4 u_v_length);



TLORACLE_DLL sword SDO_OCITypeVTSelect(OCIEnv *env, OCIError *err, CONST text *schema_name,

						  ub4 s_n_length, CONST text *type_name, ub4 t_n_length, 

						  text **user_version, ub4 *u_v_length, ub2 *version);
//@}

#endif 




