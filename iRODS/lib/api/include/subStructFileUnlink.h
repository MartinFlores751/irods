/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to subStructFiles in the COPYRIGHT directory ***/
/* subStructFileUnlink.h  
 */

#ifndef SUB_STRUCT_FILE_UNLINK_H
#define SUB_STRUCT_FILE_UNLINK_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_UNLINK rsSubStructFileUnlink
/* prototype for the server handler */
int
rsSubStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile);
int
_rsSubStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile);
int
remoteSubStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_UNLINK NULL
#endif

/* prototype for the client call */
int
rcSubStructFileUnlink (rcComm_t *conn, subFile_t *subFile);

#endif	/* SUB_STRUCT_FILE_UNLINK_H */
