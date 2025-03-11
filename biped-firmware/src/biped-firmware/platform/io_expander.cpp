/**
 *  @file   io_expander.cpp
 *  @author Simon Yu
 *  @date   12/06/2022
 *  @brief  I/O expander class source.
 *
 *  This file implements the I/O expander class.
 */

/*
 *  External headers.
 */
#include <esp32-hal-gpio.h>

/*
 *  Project headers.
 */
#include "common/global.h"
#include "common/parameter.h"
#include "platform/io_expander.h"
#include "platform/serial.h"

/*
 *  Biped namespace.
 */
namespace biped
{
/*
 *  Firmware namespace.
 */
namespace firmware
{
IOExpander::IOExpander(const uint8_t& address) :
        interrupt_handlers_port_a_(IOExpanderParameter::port_pin_count),
        interrupt_handlers_port_b_(IOExpanderParameter::port_pin_count)
{
    /*
     *  Using the C++ STL std::make_shared function, instantiate Arduino
     *  I/O expander driver object with the given address.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
//    biped::firmware::Serial(biped::firmware::Serial::getLogLevelWorst()) << "IOExpander constructor.";
    mcp23018_ = std::make_shared<MCP23018>(address);

    /*
     *  Initialize Arduino I/O expander driver object.
     *
     *  Refer to the MCP23018 header for the driver functions.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->begin();

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver writeToRegister function in the MCP23018
     *  header, set the mirror bit (IOCON_MIRROR), the interrupt polarity bit
     *  (IOCON_INTPOL), and the interrupt clearing control bit (IOCON_INTCC) in
     *  the I/O expander control register (IOCON).
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->writeToRegister(IOCON_MIRROR, 1);
    mcp23018_->writeToRegister(IOCON_INTPOL, 1);
    mcp23018_->writeToRegister(IOCON_INTCC, 1);

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver SetDirections function in the MCP23018
     *  header, initialize all pins on both port A and B as input pins.
     *
     *  Note that the SetDirections sets the I/O direction registers (IODIR).
     *  The first argument sets the port A IODIR register, and the second argument
     *  sets the port B IODIR register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->SetDirections(1, 1);

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver SetPullups function in the MCP23018
     *  header, enable pull-up resistors on all pins on both ports A and B.
     *
     *  Note that the SetPullups sets the GPIO pull-up resistor registers (GPPU).
     *  The first argument sets the port A GPPU register, and the second argument
     *  sets the port B GPPU register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->SetPullups(1, 1);

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver writePairToRegister function in the
     *  MCP23018 header, set the interrupt-on-change control registers (INTEN,
     *  GPINTEN in the datasheet) for both port A and B such that interrupts
     *  (interrupt-on-change-events) on all pins are disabled.
     *
     *  Note that the writePairToRegister sets port A and port B registers at
     *  once. The first argument should always be the port A register address
     *  as the writePairToRegister function internally extrapolates the port B
     *  register address from the given port A register address. The second
     *  argument corresponds to the value for the port A register and the third
     *  argument corresponds to the value for the port B register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->writePairToRegister(INTENA, 0, 0);
}

std::shared_ptr<MCP23018>
IOExpander::get() const
{
    /*
     *  Return the class member Arduino I/O expander driver shared pointer.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    return mcp23018_;
}

MCP23018*
IOExpander::getRaw() const
{
    /*
     *  Using the class member Arduino I/O expander driver shared pointer,
     *  return the Arduino I/O expander driver raw pointer.
     *
     *  Learn more about the C++ STL std::shared_ptr here:
     *  https://cplusplus.com/reference/memory/shared_ptr/
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    return mcp23018_.get();
}

void
IOExpander::attachInterruptPortA(const uint8_t& pin, void
(*handler)(void*), void* arg, const int& mode)
{
    /*
     *  If the given interrupt mode is disabled, detach all interrupt handlers
     *  from the given pin on this port and return.
     *
     *  Refer to the esp32-hal-gpio header for the available interrupt modes
     *  (Interrupt Modes).
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    if (mode == DISABLED) {
        detachInterruptPortA(pin);
        return;
    }

    /*
     *  Validate the given pin.
     */
    if (pin >= IOExpanderParameter::port_pin_count)
    {
        Serial(LogLevel::error) << "Invalid pin.";
        return;
    }

    /*
     *  Register the interrupt handler by assigning the given interrupt handler
     *  function pointer, the given interrupt handler function argument pointer
     *  the given interrupt mode to the interrupt handler struct for the given
     *  pin in the class member interrupt handler vector for this port.
     *
     *  Refer to the I/O expander header for the interrupt handler struct.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    InterruptHandler temp;
    temp.handler = handler;
    temp.arg = arg;
    temp.mode = mode;
    interrupt_handlers_port_a_[pin] = temp;

    /*
     *  Switch on the given interrupt mode.
     */
    switch (mode)
    {
        case RISING:
        case FALLING:
        case CHANGE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only edge-triggered interrupts (rising,
             *  falling, and change) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONA, pin, false);

            break;
        }
        case ONLOW:
        case ONLOW_WE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only level-triggered interrupts (on-low,
             *  on-high, etc.) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONA, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the default value register (DEFVAL) for the
             *  given pin on this port such that only low level-triggered interrupts
             *  (on-low and on-low-wakeup-enable) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(DEFVALA, pin, true);

            break;
        }
        case ONHIGH:
        case ONHIGH_WE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only level-triggered interrupts (on-low,
             *  on-high, etc.) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONA, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the default value register (DEFVAL) for the
             *  given pin on this port such that only high level-triggered interrupts
             *  (on-high and on-high-wakeup-enable) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(DEFVALA, pin, false);

            break;
        }
        default:
        {
            /*
             *  Print a warning message to serial for unsupported interrupt mode.
             */
            Serial(LogLevel::warn) << "Unsupported interrupt mode.";
            break;
        }
    }

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver setBitInRegister function in the
     *  MCP23018 header, set the interrupt-on-change control register (INTEN,
     *  GPINTEN in the datasheet) for the given pin on this port such that
     *  interrupts (interrupt-on-change-events) on the given pin is enabled.
     *
     *  Note that the setBitInRegister sets a single bin in the given register
     *  address. The first argument corresponds to the register address for one
     *  port, the second argument corresponds to the bit to be set in the given
     *  register (i.e., the pin number), and the third argument corresponds to
     *  the value of the bit to be set in the given register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->setBitInRegister(INTENA, pin, true);
}

void
IOExpander::attachInterruptPortB(const uint8_t& pin, void
(*handler)(void*), void* arg, const int& mode)
{
    /*
     *  If the given interrupt mode is disabled, detach all interrupt handlers
     *  from the given pin on this port and return.
     *
     *  Refer to the esp32-hal-gpio header for the available interrupt modes
     *  (Interrupt Modes).
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    if (mode == DISABLED) {
        detachInterruptPortB(pin);
        return;
    }


    /*
     *  Validate the given pin.
     */
    if (pin >= IOExpanderParameter::port_pin_count)
    {
        Serial(LogLevel::error) << "Invalid pin.";
        return;
    }

    /*
     *  Register the interrupt handler by assigning the given interrupt handler
     *  function pointer, the given interrupt handler function argument pointer
     *  the given interrupt mode to the interrupt handler struct for the given
     *  pin in the class member interrupt handler vector for this port.
     *
     *  Refer to the I/O expander header for the interrupt handler struct.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    InterruptHandler temp;
    temp.handler = handler;
    temp.arg = arg;
    temp.mode = mode;
    interrupt_handlers_port_b_[pin] = temp;

    /*
     *  Switch on the given interrupt mode.
     */
    switch (mode)
    {
        case RISING:
        case FALLING:
        case CHANGE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only edge-triggered interrupts (rising,
             *  falling, and change) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONB, pin, false);

            break;
        }
        case ONLOW:
        case ONLOW_WE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only level-triggered interrupts (on-low,
             *  on-high, etc.) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONB, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the default value register (DEFVAL) for the
             *  given pin on this port such that only low level-triggered interrupts
             *  (on-low and on-low-wakeup-enable) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(DEFVALB, pin, true);

            break;
        }
        case ONHIGH:
        case ONHIGH_WE:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the interrupt control register (INTCON) for the
             *  given pin on this port such that only level-triggered interrupts (on-low,
             *  on-high, etc.) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(INTCONB, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the default value register (DEFVAL) for the
             *  given pin on this port such that only high level-triggered interrupts
             *  (on-high and on-high-wakeup-enable) are generated.
             *
             *  Learn more about level-triggered vs. edge-triggered interrupts here:
             *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Before generating an interrupt for a given pin, the MCP23018 I/O expander
             *  compares the given pin state against one of the two sources, either against
             *  the bit associated with the given pin in the default value register (DEFVAL)
             *  for this port or against the previous pin state. The MCP23018 I/O expander
             *  only generates an interrupt for the given pin if the given pin state is
             *  different from the comparison source. The interrupt control register (INTCON)
             *  selects the comparison source.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(DEFVALB, pin, false);

            break;
        }
        default:
        {
            /*
             *  Print a warning message to serial for unsupported interrupt mode.
             */
            Serial(LogLevel::warn) << "Unsupported interrupt mode.";
            break;
        }
    }

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver setBitInRegister function in the
     *  MCP23018 header, set the interrupt-on-change control register (INTEN,
     *  GPINTEN in the datasheet) for the given pin on this port such that
     *  interrupts (interrupt-on-change-events) on the given pin is enabled.
     *
     *  Note that the setBitInRegister sets a single bin in the given register
     *  address. The first argument corresponds to the register address for one
     *  port, the second argument corresponds to the bit to be set in the given
     *  register (i.e., the pin number), and the third argument corresponds to
     *  the value of the bit to be set in the given register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->setBitInRegister(INTENB, pin, true);
}

void
IOExpander::detachInterruptPortA(const uint8_t& pin)
{
    /*
     *  Validate the given pin.
     */
    if (pin >= IOExpanderParameter::port_pin_count)
    {
        Serial(LogLevel::error) << "Invalid pin.";
        return;
    }

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver setBitInRegister function in the
     *  MCP23018 header, set the interrupt-on-change control register (INTEN,
     *  GPINTEN in the datasheet) for the given pin on this port such that
     *  interrupts (interrupt-on-change-events) on the given pin is disabled.
     *
     *  Note that the setBitInRegister sets a single bin in the given register
     *  address. The first argument corresponds to the register address for one
     *  port, the second argument corresponds to the bit to be set in the given
     *  register (i.e., the pin number), and the third argument corresponds to
     *  the value of the bit to be set in the given register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->setBitInRegister(INTENA, pin, false);

    /*
     *  Unregister the interrupt handler by setting the interrupt handler
     *  function pointer and the interrupt handler function argument pointer
     *  for the given pin in the class member interrupt handler vector for
     *  this port to null pointers, and the interrupt mode for the given pin
     *  in the class member interrupt handler vector for this port to disabled.
     *
     *  Note that due to the existence of smart pointers in C++ (unique pointer,
     *  shared pointer, etc.) One should always use the nullptr keyword to denote
     *  all null pointer values when programming in C++, instead of the C macro NULL.
     *
     *  Refer to the I/O expander header for the interrupt handler struct and
     *  the esp32-hal-gpio header for the available interrupt modes (Interrupt
     *  Modes).
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    interrupt_handlers_port_a_[pin].handler = nullptr;
    interrupt_handlers_port_a_[pin].arg = nullptr;
    interrupt_handlers_port_a_[pin].mode = DISABLED;
}

void
IOExpander::detachInterruptPortB(const uint8_t& pin)
{
    /*
     *  Validate the given pin.
     */
    if (pin >= IOExpanderParameter::port_pin_count)
    {
        Serial(LogLevel::error) << "Invalid pin.";
        return;
    }

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver setBitInRegister function in the
     *  MCP23018 header, set the interrupt-on-change control register (INTEN,
     *  GPINTEN in the datasheet) for the given pin on this port such that
     *  interrupts (interrupt-on-change-events) on the given pin is disabled.
     *
     *  Note that the setBitInRegister sets a single bin in the given register
     *  address. The first argument corresponds to the register address for one
     *  port, the second argument corresponds to the bit to be set in the given
     *  register (i.e., the pin number), and the third argument corresponds to
     *  the value of the bit to be set in the given register.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->setBitInRegister(INTENB, pin, false);

    /*
     *  Unregister the interrupt handler by setting the interrupt handler
     *  function pointer and the interrupt handler function argument pointer
     *  for the given pin in the class member interrupt handler vector for
     *  this port to null pointers, and the interrupt mode for the given pin
     *  in the class member interrupt handler vector for this port to disabled.
     *
     *  Note that due to the existence of smart pointers in C++ (unique pointer,
     *  shared pointer, etc.) One should always use the nullptr keyword to denote
     *  all null pointer values when programming in C++, instead of the C macro NULL.
     *
     *  Refer to the I/O expander header for the interrupt handler struct and
     *  the esp32-hal-gpio header for the available interrupt modes (Interrupt
     *  Modes).
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    interrupt_handlers_port_b_[pin].handler = nullptr;
    interrupt_handlers_port_b_[pin].arg = nullptr;
    interrupt_handlers_port_b_[pin].mode = DISABLED;
}

void
IOExpander::pinModePortA(const uint8_t& pin, const uint8_t& mode)
{
//    biped::firmware::Serial(LogLevel::trace) << "In pinModePortA.";
    switch (mode)
    {
        case INPUT:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an input.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRA, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is disabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUA, pin, false);

            break;
        }
        case INPUT_PULLUP:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an input.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRA, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is enabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUA, pin, true);

            break;
        }
        case OUTPUT:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an output.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRA, pin, false);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is enabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUA, pin, true);

            break;
        }
        default:
        {
            /*
             *  Print an error message to serial for unknown pin mode.
             */
            Serial(LogLevel::error) << "Unknown pin mode.";
            break;
        }
    }
}

void
IOExpander::pinModePortB(const uint8_t& pin, const uint8_t& mode)
{
//    biped::firmware::Serial(LogLevel::trace) << "In pinModePortB.";
    switch (mode)
    {
        case INPUT:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an input.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRB, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is disabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUB, pin, false);

            break;
        }
        case INPUT_PULLUP:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an input.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRB, pin, true);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is enabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUB, pin, true);

            break;
        }
        case OUTPUT:
        {
            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the I/O direction register (IODIR) for the given
             *  pin on this port such that the given pin is configured as an output.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(IODIRB, pin, false);

            /*
             *  Using the class member Arduino I/O expander driver shared pointer and
             *  the Arduino I/O expander driver setBitInRegister function in the
             *  MCP23018 header, set the GPIO pull-up resistor register (GPPU) for the
             *  given pin on this port such that the pull-up resistor for the given pin
             *  is enabled.
             *
             *  Note that the setBitInRegister sets a single bin in the given register
             *  address. The first argument corresponds to the register address for one
             *  port, the second argument corresponds to the bit to be set in the given
             *  register (i.e., the pin number), and the third argument corresponds to
             *  the value of the bit to be set in the given register.
             *
             *  Refer to the MCP23018 header for the driver functions and the register
             *  macro definitions, and the MCP23018 I/O expander datasheet for details
             *  on the I/O expander registers here:
             *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
             *
             *  TODO LAB 4 YOUR CODE HERE.
             */
            mcp23018_->setBitInRegister(GPPUB, pin, true);

            break;
        }
        default:
        {
            /*
             *  Print an error message to serial for unknown pin mode.
             */
            Serial(LogLevel::error) << "Unknown pin mode.";
            break;
        }
    }
}

bool
IOExpander::digitalReadPortA(const uint8_t& pin)
{
    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver GetPort functions in the MCP23018 header,
     *  read pin states for this port into an 8-bit unsigned integer (uint8_t).
     *  Remember to static_cast the return value of the GetPort functions into
     *  uint8_t as the GetPort functions return int.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  Note that the GetPort functions read the states of all pins on this port
     *  as an 8-bit unsigned integer where each bit corresponds to the state of
     *  each pin on this port.
     *
     *  Refer to the MCP23018 header for the driver functions.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint8_t port_a_bits = static_cast<uint8_t>(mcp23018_->GetPortA());

    /*
     *  Extract and return the state of the given pin from the 8-bit pin states
     *  above. Remember to static_cast the extracted pin state into a boolean.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    return static_cast<bool>((port_a_bits >> pin) & 1);
}

bool
IOExpander::digitalReadPortB(const uint8_t& pin)
{
    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver GetPort functions in the MCP23018 header,
     *  obtain pin states for this port into an 8-bit unsigned integer (uint8_t).
     *  Remember to static_cast the return value of the GetPort functions into
     *  uint8_t as the GetPort functions return int.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  Note that the GetPort functions read the states of all pins on this port
     *  as an 8-bit unsigned integer where each bit corresponds to the state of
     *  each pin on this port.
     *
     *  Refer to the MCP23018 header for the driver functions.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint8_t port_b_bits = static_cast<uint8_t>(mcp23018_->GetPortB());

    /*
     *  Extract and return the state of the given pin from the 8-bit pin states
     *  above. Remember to static_cast the extracted pin state into a boolean.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    return static_cast<bool>((port_b_bits >> pin) & 1);
}

void
IOExpander::digitalWritePortA(const uint8_t& pin, const bool& state)
{
    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver SetPin functions in the MCP23018 header,
     *  set the pin state of the given pin on this port with the given state.
     *
     *  Refer to the MCP23018 header for the driver functions.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->SetAPin(pin, state);
}

void
IOExpander::digitalWritePortB(const uint8_t& pin, const bool& state)
{
    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver SetPin functions in the MCP23018 header,
     *  set the pin state of the given pin on this port with the given state.
     *
     *  Refer to the MCP23018 header for the driver functions.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    mcp23018_->SetBPin(pin, state);
}

void IRAM_ATTR
IOExpander::onInterrupt()
{
    /*
     *  Lock the global I2C driver mutex lock such that the I/O expander
     *  register reads below as atomic in this I/O expander interrupt service
     *  interrupt context. Note that all I/O expander register operations are
     *  via the I2C bus.
     *
     *  Learn more about C++ STL std::unique_lock here:
     *  https://cplusplus.com/reference/mutex/unique_lock/
     *
     *  Learn more about the I2C bus here:
     *  https://www.ti.com/lit/an/slva704/slva704.pdf
     *
     *  Refer to the global header for the global variables.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    lock_wire_.lock();

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver readPairFromRegister function in the
     *  MCP23018 header, read the interrupt flag registers (INTF) for both
     *  port A and B into a 16-bit unsigned integer (uint16_t). Remember to
     *  static_cast the return value of the readPairFromRegister function into
     *  uint16_t as the readPairFromRegister function returns int.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  Note that the readPairFromRegister reads port A and port B registers at
     *  once (8-bit each). The argument should always be the port A register address
     *  as the readPairFromRegister function internally extrapolates the port B
     *  register address from the given port A register address.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint16_t flags = static_cast<uint16_t>(mcp23018_->readPairFromRegister(INTFA));

    /*
     *  Using the class member Arduino I/O expander driver shared pointer and
     *  the Arduino I/O expander driver readPairFromRegister function in the
     *  MCP23018 header, read the interrupt capture registers (INTCAP) for both
     *  port A and B into a 16-bit unsigned integer (uint16_t). Remember to
     *  static_cast the return value of the readPairFromRegister function into
     *  uint16_t as the readPairFromRegister function returns int.
     *
     *  Note that one should always use C++ explicit type casts (static_cast,
     *  dynamic_cast, etc.) when programming in C++ instead of the C-style
     *  type cast, as the C++ type casts are checked by the compiler, whereas
     *  C-style casts are not and can fail at runtime.
     *
     *  Learn more about C++ type casting here:
     *  https://cplusplus.com/doc/oldtutorial/typecasting/
     *
     *  Note that the readPairFromRegister reads port A and port B registers at
     *  once (8-bit each). The argument should always be the port A register address
     *  as the readPairFromRegister function internally extrapolates the port B
     *  register address from the given port A register address.
     *
     *  Remember the interrupt clearing control bit (IOCON_INTCC) set to the I/O
     *  expander control register (IOCON) in the constructor. With INTCC set, the
     *  I/O expander interrupt is cleared when the INTCAP register is read.
     *  Therefore, the I/O expander interrupt is cleared beyond this line.
     *
     *  Refer to the MCP23018 header for the driver functions and the register
     *  macro definitions, and the MCP23018 I/O expander datasheet for details
     *  on the I/O expander registers here:
     *  https://ww1.microchip.com/downloads/en/devicedoc/22103a.pdf
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint16_t captures = static_cast<uint16_t>(mcp23018_->readPairFromRegister(INTCAPA));

    /*
     *  Unlock the global I2C driver mutex lock after the I/O expander register reads
     *  above. Note that all I/O expander register operations are via the I2C bus.
     *
     *  Learn more about C++ STL std::unique_lock here:
     *  https://cplusplus.com/reference/mutex/unique_lock/
     *
     *  Learn more about the I2C bus here:
     *  https://www.ti.com/lit/an/slva704/slva704.pdf
     *
     *  Refer to the global header for the global variables.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    lock_wire_.unlock();

    /*
     *  Split the 16-bit interrupt flags read above into two 8-bit unsigned integers
     *  (uint8_t), one for port A and one for port B. The lower 8 bits of the 16-bit
     *  interrupt flags correspond to port A, and the higher 8 bits correspond to port B.
     *
     *  Refer to the parameter header for the number of I/O expander pins per port.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint8_t aflags = static_cast<uint8_t>(flags);
    uint8_t bflags = static_cast<uint8_t>(flags >> 8);

    /*
     *  Split the 16-bit interrupt captures read above into two 8-bit unsigned integers
     *  (uint8_t), one for port A and one for port B. The lower 8 bits of the 16-bit
     *  interrupt captures correspond to port A, and the higher 8 bits correspond to
     *  port B.
     *
     *  Refer to the parameter header for the number of I/O expander pins per port.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    uint8_t acaptures = static_cast<uint8_t>(captures);
    uint8_t bcaptures = static_cast<uint8_t>(captures >> 8);

    /*
     *  Service the port A and port B interrupts using the split 8-bit values above and
     *  the class member interrupt handler vectors.
     *
     *  TODO LAB 4 YOUR CODE HERE.
     */
    serviceInterrupt(aflags, acaptures, interrupt_handlers_port_a_);
    serviceInterrupt(bflags, bcaptures, interrupt_handlers_port_b_);
}

void
IOExpander::serviceInterrupt(const uint8_t& flags, const uint8_t& captures,
        const std::vector<InterruptHandler>& interrupt_handlers)
{
    /*
     *  Declare pin mask and set to 1.
     */
//    biped::firmware::Serial(LogLevel::trace) << "In interrupt service.";
    uint8_t pin_mask = 1;

    for (uint8_t pin = 0; pin < IOExpanderParameter::port_pin_count; pin ++, pin_mask <<= 1)
    {
        /*
         *  Apply the pin mask to the given interrupt flags. Skip the current iteration if
         *  the interrupt flag for the current pin is not high, i.e., no interrupt occurred
         *  for the current pin.
         *
         *  TODO LAB 4 YOUR CODE HERE.
         */
        uint8_t curr_flag = (flags & pin_mask) >> pin;
        if (!curr_flag) {
            continue;
        }

        /*
         *  If the interrupt handler function pointer for the current pin in the given
         *  interrupt handler vector is a null pointer, skip the current iteration.
         *
         *  Refer to the I/O expander header for the interrupt handler struct.
         *
         *  TODO LAB 4 YOUR CODE HERE.
         */
        if (!interrupt_handlers[pin].handler) {
            continue;
        }

        /*
         *  Switch on interrupt mode.
         */
        switch (interrupt_handlers.at(pin).mode)
        {
            case RISING:
            {
                /*
                 *  Apply the pin mask to the given interrupt captures. Call the interrupt
                 *  handler for the current pin in the given interrupt handler vector if
                 *  the interrupt capture for the current pin is high, i.e., the state of
                 *  the current pin was captured as rising when its interrupt occurred.
                 *
                 *  Remember to pass the interrupt handler function argument pointer for
                 *  the current pin in the given interrupt handler vector to the interrupt
                 *  handler when calling.
                 *
                 *  Refer to the I/O expander header for the interrupt handler struct.
                 *
                 *  TODO LAB 4 YOUR CODE HERE.
                 */
                uint8_t curr_capture = (captures & pin_mask) >> pin;
                if (curr_capture) {
                    interrupt_handlers[pin].handler(interrupt_handlers[pin].arg);
                }

                break;
            }
            case FALLING:
            {
                /*
                 *  Apply the pin mask to the given interrupt captures. Call the interrupt
                 *  handler for the current pin in the given interrupt handler vector if
                 *  the interrupt capture for the current pin is low, i.e., the state of
                 *  the current pin was captured as falling when its interrupt occurred.
                 *
                 *  Remember to pass the interrupt handler function argument pointer for
                 *  the current pin in the given interrupt handler vector to the interrupt
                 *  handler when calling.
                 *
                 *  Refer to the I/O expander header for the interrupt handler struct.
                 *
                 *  TODO LAB 4 YOUR CODE HERE.
                 */
                uint8_t curr_capture = (captures & pin_mask) >> pin;
                if (!curr_capture) {
                    interrupt_handlers[pin].handler(interrupt_handlers[pin].arg);
                }

                break;
            }
            default:
            {
                /*
                 *  Call the interrupt handler for the current pin in the given interrupt
                 *  handler vector regardless of the interrupt capture values, i.e., the
                 *  interrupt is handled for both rising and falling pin states (change
                 *  interrupt mode), or for level-triggered interrupts (on-high, on-low, etc.)
                 *
                 *  Learn more about level-triggered vs. edge-triggered interrupts here:
                 *  https://www.garystringham.com/level-triggered-vs-edge-triggered-interrupts/
                 *
                 *  Remember to pass the interrupt handler function argument pointer for
                 *  the current pin in the given interrupt handler vector to the interrupt
                 *  handler when calling.
                 *
                 *  Refer to the I/O expander header for the interrupt handler struct.
                 *
                 *  TODO LAB 4 YOUR CODE HERE.
                 */
                interrupt_handlers[pin].handler(interrupt_handlers[pin].arg);

                break;
            }
        }
    }
//    biped::firmware::Serial(LogLevel::trace) << "Exit service.";
}
}   // namespace firmware
}   // namespace biped
