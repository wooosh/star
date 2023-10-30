(map (runo-xfrm) (glob (partial string/has-suffix? ".runo")))

(copy-out "assets" "assets")
(copy-out "CNAME" "CNAME")
(copy-out "style.css" "style.css")

(use-subdir "projects")
(use-subdir "theming")
(use-subdir "misc")
(use-subdir "feed")

# TODO: fix deserted fonts