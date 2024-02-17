#
# Query the system for CRM/AU identity information
#
# Use crm_id if defined, else create a timestamp 
# Run 'git describe' for the latest tag
#
def get_git_latest_tag(path, d):
    if os.path.exists(path) :
        f = os.popen("cd %s; git describe --always 2>&1" % path)
        data = f.read()
        if f.close() is None:
            rev = data.split(" ")[0]
            if len(rev) != 0:
                return rev.rstrip("\n")
    return time.strftime('%Y%m%d%H%M')


def get_git_latest_tag_exact_match(path, d):
    if os.path.exists(path) :
        f = os.popen("cd %s; git describe --exact-match --abbrev=0 -- 2>&1" % path)
        data = f.read()
        if f.close() is None:
            rev = data.split(" ")[0]
            if len(rev) != 0:
                return rev.rstrip("\n")
    return time.strftime('%Y%m%d%H%M')
