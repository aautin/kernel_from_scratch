#pragma once

//
// Register the non-fatal CPU trap handlers. Fatal exceptions are registered
// separately by panic_init().
//
void exception_init();
