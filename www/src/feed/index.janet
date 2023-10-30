# TODO: depend on binary
(defn twtxt/buildfn
    [out-path [in-path tmpl] &]
    (spawn ["twtxt-tmpl" in-path tmpl out-path] :p))

(defn twtxt/build
    [in-path tmpl out-path]
    (cas/link (cas/id-of twtxt/buildfn [in-path tmpl]) out-path))

(twtxt/build "feed.twtxt" "atom.tmpl" (path/join dir-out "feed.atom"))
(twtxt/build "feed.twtxt" "html.tmpl" (path/join dir-out "feed.html"))