(module
  (func $sum (param $n i32) (result i32)
    (local $i i32)
    (local $sum i32)
    
    i32.const 0
    local.set $sum
    i32.const 1
    local.set $i
    
    (block $exit
      (loop $loop
        local.get $i
        local.get $n
        i32.gt_u
        br_if $exit

        local.get $sum 
        local.get $i
        i32.add          
        local.set $sum
        

        local.get $i
        i32.const 1
        i32.add
        local.set $i
        
        (br $loop)
      )
    )

    (local.get $sum)
  )

  (export "sum" (func $sum))
)