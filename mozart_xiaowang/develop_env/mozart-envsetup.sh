function mozart-gettop()
{
    local TOPFILE=develop_env/mozart-envsetup.sh
    if [ -n "$MOZART_TOPDIR" -a -f "$MOZART_TOPDIR/$TOPFILE" ] ; then
        echo $MOZART_TOPDIR
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            # We redirect cd to /dev/null in case it's aliased to
            # a command that prints something as a side-effect
            # (like pushd)
            local HERE=$PWD
            T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
                cd .. > /dev/null
                T=`PWD= /bin/pwd`
            done
            cd $HERE > /dev/null
            if [ -f "$T/$TOPFILE" ]; then
                echo $T
            fi
        fi
    fi
}

function mozart-get-dir()
{
    T=$(mozart-gettop)
    if [ "$T" ]; then
        echo "$T $T/src/vr $T/output/molib"
    fi
}

function mozart-repo-sync-speaker-master()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)
    if [ "$T" ]; then
        M=`readlink $T/../.repo/manifest.xml`
        if [ "$M"x = "manifests/master.xml"x ]; then
            $T/develop_env/repo start speaker-master $D
            $T/develop_env/repo sync $D
        else
            echo "repo manifest.xml isn't master.xml. Please run \`repo init -m master.xml\`"
        fi
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function mozart-repo()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)
    if [ "$T" ]; then
        $T/develop_env/repo $@ $D
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function mozart-git()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)
    if [ "$T" ]; then
        $T/develop_env/repo forall $D -c "echo -n \"\033[45;37mproject:  \" && pwd && echo -n \"\033[0m\" && git $@"
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function mozart-create-release()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)

    if [ $# -lt 1 ]; then
        echo "Usage: mozart-create-release <branch>"
    else
        if [ "$T" ]; then
            for project in $D; do $T/develop_env/repo forall $project -c "git remote add local git@192.168.2.2:/backup/02Bu1shared/01SmartAudio_Release/codes/mozart/`basename $project`"; done
            mozart-git "checkout -b release-$1"
            mozart-git "push local HEAD:release-$1"
        else
            echo "Couldn't locate the top of the tree.  Try setting TOP."
        fi
    fi
}

function mozart-checkout-release()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)

    if [ $# -lt 1 ]; then
        echo "Usage: mozart-checkout-release <branch>"
    else
        if [ "$T" ]; then
            for project in $D; do $T/develop_env/repo forall $project -c "git remote add local git@192.168.2.2:/backup/02Bu1shared/01SmartAudio_Release/codes/mozart/`basename $project`"; done
            mozart-git "fetch local"
            mozart-git "checkout -b release-$1 --track local/release-$1"
        else
            echo "Couldn't locate the top of the tree.  Try setting TOP."
        fi
    fi
}

function mozart-merge-release()
{
    T=$(mozart-gettop)
    D=$(mozart-get-dir)

    if [ $# -lt 1 ]; then
        echo "Usage: mozart-merge-release <branch>"
    else
        if [ "$T" ]; then
            mozart-git "push ing HEAD:refs/for/speaker-release"
            mozart-git "branch -D speaker-master-tmp"
            mozart-git "checkout -b speaker-master-tmp --track ing/speaker-master"
            mozart-git "pull"
            mozart-git "merge --no-ff release-$1"
            mozart-git "push ing HEAD:refs/for/speaker-master"
        else
            echo "Couldn't locate the top of the tree.  Try setting TOP."
        fi
    fi
}

function mozart-croot()
{
    T=$(mozart-gettop)
    if [ "$T" ]; then
        cd $(mozart-gettop)
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

cat <<EOF
Invoke "source develop_env/mozart-envsetup.sh" from your shell to add the following functions to your environment:
- mozart-croot:  Changes directory to the top of the tree.
- mozart-git:    repo forall mozart mozart/src/vr mozart/output/molib
- mozart-repo:   repo mozart mozart/src/vr mozart/output/molib
- mozart-repo-sync-speaker-master:  repo sync mozart mozart/src/vr mozart/output/molib
- mozart-create-release:   create a release branch
- mozart-checkout-release: checkout a release branch
- mozart-merge-release:    merage a release branch

EOF
