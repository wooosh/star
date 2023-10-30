((runo-xfrm) "theming.runo")

(defn xfrm-theming-image
    [filename]
    (string "theming_" (path/basename filename)))

(let [imgs (glob (partial (partial string/has-suffix? ".png")))
    out-filenames (map xfrm-theming-image imgs)
    in-out (partition 2 (interleave imgs out-filenames))]
    (map (fn [(in-path out-path)] (copy-out in-path out-path)) in-out))