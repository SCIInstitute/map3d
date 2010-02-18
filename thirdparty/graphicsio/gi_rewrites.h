#ifndef __GI_REWWRITES_H__
#define __GI_REWRITES_H__

#ifdef __cplusplus
extern "C" {
#endif

long initrewrite(rewriteQueuePtr *thisQueue);
long addrewriterequest(rewriteQueuePtr thisQueue, rewriteRequestPtr theRequest);
long addnewrequest(rewriteQueuePtr thisQueue, rewriteRequestPtr thisRequest);
long rewritefile(rewriteQueuePtr thisQueue, char *oldFileName, char *newFileName);
rewriteRequestPtr searchList(queuedRewriteRequestPtr theRewriteQueue, long theDataType, long whichSurface, 
			     long whichOne);
rewriteRequestPtr searchAddList(queuedRewriteRequestPtr theAddQueue, long theDataType, long whichSurface);
queuedRewriteRequestPtr searchAddList2(queuedRewriteRequestPtr theAddQueue, long theDataType, long whichSurface);

#ifdef __cplusplus
}
#endif

#endif
