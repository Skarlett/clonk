use std::collections::HashMap;

extern "C" enum OpCode;


struct Object {
    attributes: HashMap<
    
}

enum Datatype {
    Byte(u8),
    Double(u16),
    SignedDouble(i16),

    Integer(isize),
    UnsignedInteger(usize),

    String(String),
}

struct Registers {
    r0: Datatype,
    r1: Datatype
}

struct VirtMachineState {
    stack: Vec<>
    heap: HashMap<usize, Datatype>
}


fn start() {

}