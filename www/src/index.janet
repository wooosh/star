(defn build-all-runo
    []
    (each file (glob [:suffix ".runo"])
    (runo/build file :dir dir-out)))

(defn build-all-html
    []
    (each file (glob [:suffix ".html"])
    (copy file (path/join dir-out file))))

(defn default-build
    [dir]
    (def prevdir (os/cwd))
    (os/cd dir)
    (build-all-html)
    (build-all-runo)
    (os/cd prevdir))

(each file ["assets" "CNAME" "style.css"]
    (copy file (path/join dir-out file)))

(each dir ["." "projects" "misc"]
    (default-build dir))

(use-subdir "theming")
(use-subdir "feed")