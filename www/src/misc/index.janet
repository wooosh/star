(map (runo-xfrm) (glob (partial string/has-suffix? ".runo")))
(map (fn [f] (copy-out f (path/basename f)))
    (glob (partial string/has-suffix? ".html")))