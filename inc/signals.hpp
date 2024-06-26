#pragma once

#include <csignal>
#include <iostream>

extern volatile	sig_atomic_t globalSignalReceived;

void	initSignals();