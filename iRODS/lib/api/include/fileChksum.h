/* -*- mode: c++; fill-column: 132; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* fileChksum.h - This file may be generated by a program or script
 */

#ifndef FILE_CHKSUM_H
#define FILE_CHKSUM_H

/* This is a low level file type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"

#include "fileDriver.h"

typedef struct FileChksumInp {
    fileDriverType_t fileType;
    rodsHostAddr_t addr;
    char fileName[MAX_NAME_LEN];
    char rescHier[MAX_NAME_LEN];
    char objPath[MAX_NAME_LEN];
    int flag;   /* not used for now */
    char in_pdmo[MAX_NAME_LEN]; // Flag indicating if we are being executed from a pdmo
} fileChksumInp_t;
    
#define fileChksumInp_PI "int fileType; struct RHostAddr_PI; str fileName[MAX_NAME_LEN]; str rescHier[MAX_NAME_LEN]; str objPath[MAX_NAME_LEN]; int flags; str in_pdmo[MAX_NAME_LEN];"

#define fileChksumOut_PI "str chksumStr[NAME_LEN];"

#if defined(RODS_SERVER)
#define RS_FILE_CHKSUM rsFileChksum
/* prototype for the server handler */
int
rsFileChksum (rsComm_t *rsComm, fileChksumInp_t *fileChksumInp, 
              char **chksumStr);
int
_rsFileChksum (rsComm_t *rsComm, fileChksumInp_t *fileChksumInp,
               char **chksumStr);
int
remoteFileChksum (rsComm_t *rsComm, fileChksumInp_t *fileChksumInp,
                  char **chksumStr, rodsServerHost_t *rodsServerHost);
int
fileChksum (int fileType, rsComm_t *rsComm, char* objPath, char *fileName, char* rescHier, char *chksumStr);
#else
#define RS_FILE_CHKSUM NULL
#endif

/* prototype for the client call */
int
rcFileChksum (rcComm_t *conn, fileChksumInp_t *fileChksumInp,
              char **chksumStr);

#endif  /* FILE_CHKSUM_H */
