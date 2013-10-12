/* -*- mode: c++; fill-column: 132; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* fileTruncate.h - This file may be generated by a program or script
 */

#ifndef FILE_TRUNCATE_H
#define FILE_TRUNCATE_H

/* This is a Internal I/O API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "fileOpen.h"

#include "fileDriver.h"

#if defined(RODS_SERVER)
#define RS_FILE_TRUNCATE rsFileTruncate
/* prototype for the server handler */
int
rsFileTruncate (rsComm_t *rsComm, fileOpenInp_t *fileTruncateInp);
int
_rsFileTruncate (rsComm_t *rsComm, fileOpenInp_t *fileTruncateInp);
int
remoteFileTruncate (rsComm_t *rsComm, fileOpenInp_t *fileTruncateInp,
                    rodsServerHost_t *rodsServerHost);
#else
#define RS_FILE_TRUNCATE NULL
#endif

/* prototype for the client call */
int
rcFileTruncate (rcComm_t *conn, fileOpenInp_t *fileTruncateInp);

#endif  /* FILE_TRUNCATE_H */

