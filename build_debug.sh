#!/bin/bash
make DEFINES=-DMAKE_DEBUG CFLAGS+"=-g -fsanitize=address" LDFLAGS+=-fsanitize=address all
