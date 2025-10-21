#ifndef COMMON_H
#define COMMON_H

#define KERNELSIM_VERSION "0.5.0"

#define FIFO_SYSREQ "/tmp/ksim_sysreq.fifo"
#define FIFO_IRQ    "/tmp/ksim_irq.fifo"

#define IRQ_TIMER  0
#define IRQ_D1     1
#define IRQ_D2     2

#define OP_R  'R'
#define OP_W  'W'
#define OP_X  'X'
#endif