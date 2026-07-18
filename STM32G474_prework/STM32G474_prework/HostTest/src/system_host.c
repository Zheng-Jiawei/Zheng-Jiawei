/*
 * Host-only replacement for the STM32 device startup routine.
 *
 * The Keil Cortex-M core simulator used by HRC_HostTest has no STM32G474
 * RCC/FLASH/GPIO peripheral model.  Do not use system_stm32g4xx.c in this
 * target: its register accesses can raise a simulated fault before main().
 */

void SystemInit(void)
{
    /* Intentionally empty: the host model initializes virtual peripherals. */
}
