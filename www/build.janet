(import spork/path)
(import spork/sh)

# TODO: use (default) more

# configuration
(def dir-src (path/abspath "src"))
(def dir-out (path/abspath "root"))

# utility functions
# TODO: make better use of keyword args
# TODO: glob :suffix abc
# TODO: glob function
(defn glob
    "Returns an array of files matching the predicate"
    [predicate &opt dir]
    (filter predicate (os/dir (path/abspath (or dir ".")))))

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

# build functions
(def runo-tmpl (path/abspath "page.tmpl"))
(defn runo
    [in-file out-file]
    (assert (= 0 (os/proc-wait (os/spawn ["runo" runo-tmpl in-file] :p {:out (file/open out-file :wn)})))))

(defn twtxt-tmpl
    [in-file tmpl out-file]
    (assert (= 0 (os/proc-wait (os/spawn ["twtxt-tmpl" in-file tmpl out-file] :p {:out (file/open out-file :wn)})))))

(defn runo-xfrm
    [&opt out-path]
    (let [out-path (or out-path ".")]
        (fn [x] (runo x (as-> x _
                            (path/basename _)
                            (del-suffix ".runo" _)
                            (string _ ".html")
                            (path/join dir-out out-path _))))))

(defn copy-out
    [in-path out-path]
    (sh/copy in-path (path/join dir-out out-path)))

# execute the build
#(glob (partial string/has-suffix? ".runo"))
(def dir-src (path/abspath dir-src))
(def dir-out (path/abspath dir-out))

(sh/rm dir-out)
(os/mkdir dir-out)

(use-subdir dir-src)