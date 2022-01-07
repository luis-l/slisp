# slisp
C++ implementation of LISP

Example:

```lisp
; String -> Int
(fun {shrekfuzz n} {
  select
    { (<= n 0) "GET OFF MY SWAP!" }
    { (eq 0 (mod n 3))  "OGRES... "  }
    { (eq 0 (mod n 5))  "... ARE LIKE ONIONS"  }
    { (eq 0 (mod n 7))  "FIONA!"  }
    { otherwise "S" }
})

(map print (map shrekfuzz (range 0 20 )))
```

For more examples, checkout the [standard library](standard/Standard.slisp).
