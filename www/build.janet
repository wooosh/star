(import spork/path)
(import spork/sh)

# the path for source directory to build
(def dir-src (path/abspath "src"))
# the path for temporary files
(def dir-tmp (path/abspath "tmp"))
# the path for the store
(def dir-store (path/abspath "store"))
# the path for the output directory
(def dir-out (path/abspath "root"))

(dofile "cas.janet" :env (curenv))

# for parallelization:
# each build function should just add a job to a queue
# have a cas/path (blocking) and cas/path-noblock
# then spawn build-fn in a coroutine and do cas/notify to get dependents to work

(defn copy
    [src dst]
    (when (path/abspath? dst)
    (def dst (if (path/abspath? dst)
                dst
                (path/join dir-out dst)))
    (assert (string/has-prefix? dir-out dst))
    (sh/copy src dst)))

(defn glob
    "Returns an array of files matching the predicate"
    [predicate &opt dir]
    (default dir ".")
    (def predicate (match predicate
        [:suffix sfx] (partial string/has-suffix? sfx)
        [:prefix pfx] (partial string/has-prefix? pfx)
        _ predicate))
    (filter predicate (os/dir (path/abspath dir))))

(defn del-suffix
    [sfx s]
    (if (string/has-suffix? sfx s)
        (string/slice s 0 (- -1 (length sfx)))
        s))

(defn use-subdir
    "Executes the index.janet file in the subdir"
    [dir]
    (os/cd dir)
    (let [dir-src (os/cwd)]
        (dofile "./index.janet" :env (curenv)))
    (os/cd dir-src))

(defn spawn
    [& args]
    (assert (= 0 (os/proc-wait (os/spawn (splice args))))))


(def runo/binary-path "runo")
(def runo/default-page-tmpl (path/abspath "page.tmpl"))

(defn runo/buildfn
    [out-filename [filename page-tmpl] &]
    (spawn ["runo" page-tmpl filename] :p {:out (file/open out-filename :wn)}))

(defn- runo/autopath
    [src dir]
    (as-> src path
        (path/basename path)
        (del-suffix ".runo" path)
        (string path ".html")
        (path/join dir path)))

(defn runo/build
    [in-path out-kind out-path &opt page-tmpl]
    (default page-tmpl runo/default-page-tmpl)
    (def out-path (match out-kind
                        :file out-path
                        :dir (runo/autopath in-path out-path)))
    (os/mkdir (path/dirname out-path))
    # TODO: add runo binary path to dependencies
    (cas/link (cas/id-of runo/buildfn [in-path page-tmpl]) out-path))

(defn do-build
    []
    # clean directories from previous build
    (sh/rm dir-tmp)
    (sh/rm dir-out)
    # ensure all neccessary directories are present
    (os/mkdir dir-tmp)
    (os/mkdir dir-out)
    (os/mkdir dir-store)
    # begin the building process
    (use-subdir dir-src))

(do-build)