(runo/build "theming.runo" :dir dir-out)

(each img (glob [:suffix ".png"])
    (copy img (path/join dir-out (string "theming_" img))))