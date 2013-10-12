/* -*- mode: c++; fill-column: 132; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See fileTruncate.h for a description of this API call.*/

#include "fileTruncate.h"
#include "miscServerFunct.h"

// =-=-=-=-=-=-=-
// eirods includes
#include "eirods_log.h"
#include "eirods_file_object.h"
#include "eirods_stacktrace.h"
#include "eirods_resource_backport.h"

int
rsFileTruncate (rsComm_t *rsComm, fileOpenInp_t *fileTruncateInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    //remoteFlag = resolveHost (&fileTruncateInp->addr, &rodsServerHost);
    eirods::error ret = eirods::get_host_for_hier_string( fileTruncateInp->resc_hier_, remoteFlag, rodsServerHost );
    if( !ret.ok() ) {
        eirods::log( PASSMSG( " failed in call to eirods::get_host_for_hier_string", ret ) );
        return -1;
    }
    if (remoteFlag == LOCAL_HOST) {
        status = _rsFileTruncate (rsComm, fileTruncateInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileTruncate (rsComm, fileTruncateInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
                     "rsFileTruncate: resolveHost returned unrecognized value %d",
                     remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    /* Manually insert call-specific code here */

    return (status);
}

int
remoteFileTruncate (rsComm_t *rsComm, fileOpenInp_t *fileTruncateInp,
                    rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
                 "remoteFileTruncate: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }


    status = rcFileTruncate (rodsServerHost->conn, fileTruncateInp);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
                 "remoteFileTruncate: rcFileTruncate failed for %s, status = %d",
                 fileTruncateInp->fileName, status);
    }

    return status;
}

// =-=-=-=-=-=-=-
// local function which makes the call to truncate via the resource plugin
int _rsFileTruncate( 
    rsComm_t*      _comm, 
    fileOpenInp_t* _trunc_inp ) {
    // =-=-=-=-=-=-=-
    // trap bad object paths
    if(_trunc_inp->objPath[0] == '\0') {
        std::stringstream msg;
        msg << __FUNCTION__;
        msg << " - Empty logical path.";
        eirods::log(LOG_ERROR, msg.str());
        return -1;
    }

    // =-=-=-=-=-=-=-
    // make the call to rename via the resource plugin
    eirods::file_object_ptr file_obj( 
                                new eirods::file_object( 
                                    _comm, 
                                    _trunc_inp->objPath, 
                                    _trunc_inp->fileName, 
                                    _trunc_inp->resc_hier_, 
                                    0, 0, 0 ) );
    file_obj->size( _trunc_inp->dataSize );
    eirods::error trunc_err = fileTruncate( _comm, file_obj );

    // =-=-=-=-=-=-=-
    // report errors if any
    if ( !trunc_err.ok() ) {
        std::stringstream msg;
        msg << "fileTruncate for [";
        msg << _trunc_inp->fileName;
        msg << "]";
        msg << trunc_err.code();
        eirods::error err = PASSMSG( msg.str(), trunc_err );
        eirods::log ( err );
    }

    return trunc_err.code();

} // _rsFileTruncate






