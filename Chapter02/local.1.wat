(module
  (type (;0;) (func))
  (type (;1;) (func (param i32)))
  (type (;2;) (func (param i64)))
  (type (;3;) (func (param i64 i32 i32)))
  (func (;0;) (type 0)
    (local i32)
    i32.const 0
    local.set 0)
  (func (;1;) (type 0)
    (local i64)
    i64.const 0
    local.set 0)
  (func (;2;) (type 1) (param i32)
    i32.const 10
    local.set 0)
  (func (;3;) (type 2) (param i64)
    i64.const 11
    local.set 0)
  (func (;4;) (type 3) (param i64 i32 i32)
    (local i64 i64)
    i64.const 0
    local.set 0
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    i64.const 0
    local.set 3
    i64.const 0
    local.set 4)
  (export "type-local-i32" (func 0))
  (export "type-local-i64" (func 1))
  (export "type-param-i32" (func 2))
  (export "type-param-i64" (func 3))
  (export "type-mixed" (func 4)))
