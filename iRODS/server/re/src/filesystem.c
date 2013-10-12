/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include <sys/stat.h>
#include <errno.h>
#include "debug.h"
#include "utils.h"
#include "datetime.h"
#include "filesystem.h"

char *getRuleBasePath(char *ruleBaseName, char rulesFileName[MAX_NAME_LEN]) {
    char *configDir = getConfigDir ();
    snprintf (rulesFileName, MAX_NAME_LEN, "%s/reConfigs/%s.re", configDir,ruleBaseName);
    return rulesFileName;

}
void getResourceName(char buf[1024], char *rname) {
	snprintf(buf, 1024, "%s/%s", getConfigDir(), rname);
	char *ch = buf;
	while(*ch != '\0') {
		if(*ch == '\\' || *ch == '/') {
			*ch = '_';
		}
		ch++;
	}

}

int getModifiedTime(char *fn, time_type *timestamp) {
	boost::filesystem::path path(fn);
	time_type time = boost::filesystem::last_write_time(path);
	time_type_set(*timestamp, time);
	return 0;
}
