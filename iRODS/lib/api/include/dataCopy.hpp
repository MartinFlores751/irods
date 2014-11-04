/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#ifndef DATA_COPY_HPP
#define DATA_COPY_HPP

/* This is a high level type API call */

#include "rods.hpp"
#include "procApiRequest.hpp"
#include "apiNumber.hpp"
#include "fileOpen.hpp"
#include "dataObjInpOut.hpp"

typedef struct DataCopyInp {
    dataOprInp_t dataOprInp;
    portalOprOut_t portalOprOut;
} dataCopyInp_t;

#define DataCopyInp_PI "struct DataOprInp_PI; struct PortalOprOut_PI;"

#if defined(RODS_SERVER)
#define RS_DATA_COPY rsDataCopy
/* prototype for the server handler */
int
rsDataCopy( rsComm_t *rsComm, dataCopyInp_t *dataCopyInp );
int
remoteDataCopy( rsComm_t *rsComm, dataCopyInp_t *dataCopyInp,
                rodsServerHost_t *rodsServerHost );
int
_rsDataCopy( rsComm_t *rsComm, dataCopyInp_t *dataCopyInp );
#else
#define RS_DATA_COPY NULL
#endif

/* prototype for the client call */
int
rcDataCopy( rcComm_t *conn, dataCopyInp_t *dataCopyInp );

#endif	/* DATA_COPY_H */
