/**
 * MAestro
 * @file halt.h
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Gracefully halts the PE
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Verifies if the PE has a pending halt request
 */
bool halt_pndg();

/**
 * @brief Tries to halt the execution
 * 
 * @details In fact, this function will stop the scheduler, free resources, and inform the mapper it is ready to indeed halt
 * 
 * @return int 0 case success, 
 * -EAGAIN if should retry
 */
int halt_try();

/**
 * @brief Sets the PE to halt
 * 
 * @param task Mapper task ID that requested the halt
 * @param addr Address of the mapper task
 * 
 * @return int 0 case success,
 * -ENOMEM if not enough memory
 */
int halt_set(int task, int addr);

/**
 * @brief Clears the halt request
 * 
 * @details This function should be called when the PE is halted
 */
void halt_clear();
