(module
  (import "console" "log" (func $log (param i32)))
  (func (export "writeFibonacciSequence") (param $n i32)
    (local $i i32)
    (local $a i32)
    (local $b i32)
    (local $next i32)
    i32.const 0
    local.set $a
    i32.const 1
    local.set $b
    i32.const 1
    local.set $i
    (block $exit
      (loop $loop
        local.get $i
        local.get $n
        i32.gt_u
        br_if $exit
        i32.const 1
        local.get $i
        i32.eq 
        (if
          (then
            i32.const 0
            call $log
            i32.const 1
            local.get $i
            i32.add
            local.set $i
            br $loop
          )
        )
        i32.const 2
        local.get $i
        i32.eq 
        (if
          (then
            i32.const 1
            call $log
            local.get $i
            i32.const 1
            i32.add
            local.set $i
            br $loop
          )
        )
        local.get $a
        local.get $b
        i32.add
        local.set $next
        local.get $b
        local.set $a
        local.get $next
        local.set $b
        local.get $next
        call $log
        local.get $i
        i32.const 1
        i32.add
        local.set $i
        br $loop
      )
    )   
  )
)
