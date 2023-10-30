(import openssl-hash)

(var hash-repr nil)

(defn- hash-repr-number
    "Get the hash representation of an object satisfying (number? x)"
    [x]
    (string ":" x ";"))

(defn- hash-repr-bytes
    "Get the hash representation of an object satisifying (bytes? x)"
    [x]
    (string (hash-repr-number (length x)) x))

(defn- hash-repr-collection
    "Get the hash representation of a tuple/array"
    [x]
    (string (hash-repr-number (length x)) (string/join (map hash-repr x))))

(varfn hash-repr
    "Get the hash representation of an object"
    [x]
    (cond (number? x) (hash-repr-number x)
          (bytes? x) (hash-repr-bytes x)
          (tuple? x) (hash-repr-collection x)
          (array? x) (hash-repr-collection x)
          (nil? x) "nil"))

(defn- hash-id
    "Return the hash id of the passed arguments"
    [& data]
    (let [h (openssl-hash/new "SHA256")]
        (openssl-hash/feed h (hash-repr data))
        (openssl-hash/finalize h :hex)))

(defn- hash-file
    [filename]
    "Return the hash of the file contents"
    (hash-id (path/abspath filename) (slurp filename)))

(defn cas/path
    "Returns the path to the artifact with the given id"
    [id]
    (path/join dir-store id))

(defn cas/has
    "Return whether or not the store has the given artifact"
    [id]
    (not (nil? (os/stat (cas/path id)))))

(defn cas/id-of
    "Return the id of the given artifact"
    [build-fn infiles &opt auxargs]
    (def infiles (map path/abspath infiles))
    (def id (hash-id
                (string build-fn)
                (splice (map hash-file infiles))
                auxargs))
    (unless (cas/has id)
        (build-fn (cas/path id) infiles auxargs))
    id)

(defn cas/link
    "Symlink a CAS object elsewhere"
    [id dst]
    (assert cas/has id)
    (os/link (cas/path id) dst true)
    (print "linking CAS object to " dst))