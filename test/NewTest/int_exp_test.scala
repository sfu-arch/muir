package concurrent

import chisel3._
import chisel3.util._
import chisel3.Module
import chisel3.testers._
import chisel3.iotesters.{ChiselFlatSpec, Driver, OrderedDecoupledHWIOTester, PeekPokeTester}
import org.scalatest.{FlatSpec, Matchers}
import muxes._
import config._
import control.{BasicBlockNoMaskNode, BasicBlockNode}
import util._
import interfaces._
import regfile._
import memory._
import stack._
import arbiters._
import loop._
import node._


class int_expTests(c: int_expDF) extends PeekPokeTester(c) {
  var cycle = 0

  def print_step(n : Int) {
    cycle += n
    step(n)
    println(s"Cycle: ${cycle}")
  }

  /**
  *  int_expDF interface:
  *
  *    data_0 = Flipped(Decoupled(new DataBundle))
  *    data_1 = Flipped(Decoupled(new DataBundle))
   *    val pred = Decoupled(new Bool())
   *    val start = Input(new Bool())
   *    val result = Decoupled(new DataBundle)
   */


  // Initializing the signals
  poke(c.io.pred.ready, true.B)

  poke(c.io.data_0.bits.data, 2.U)
  poke(c.io.data_0.bits.predicate, false.B)
  poke(c.io.data_0.bits.valid, false.B)
  poke(c.io.data_0.valid, false.B)

  poke(c.io.data_1.bits.data, 5.U)
  poke(c.io.data_1.bits.predicate, false.B)
  poke(c.io.data_1.bits.valid, false.B)
  poke(c.io.data_1.valid, false.B)

  poke(c.io.entry.bits.control, false.B)
  poke(c.io.entry.valid, false.B)

  poke(c.io.result.ready, false.B)

  print_step(1)
  poke(c.io.data_0.bits.predicate, true.B)
  poke(c.io.data_0.bits.valid, true.B)
  poke(c.io.data_0.valid, true.B)
  poke(c.io.data_1.bits.predicate, true.B)
  poke(c.io.data_1.bits.valid, true.B)
  poke(c.io.data_1.valid, true.B)
  poke(c.io.entry.bits.control, true.B)
  poke(c.io.entry.valid, true.B)

  print_step(1)
  poke(c.io.data_0.bits.valid, false.B)
  poke(c.io.data_0.valid, false.B)
  poke(c.io.data_1.bits.valid, false.B)
  poke(c.io.data_1.valid, false.B)
  poke(c.io.entry.bits.control, false.B)
  poke(c.io.entry.valid, false.B)


  while (cycle < 150) {
    print_step(1)
  }
}

class int_expTester extends FlatSpec with Matchers {
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  it should "Check that int_exp works correctly." in {
    // iotester flags:
    // -ll  = log level <Error|Warn|Info|Debug|Trace>
    // -tbn = backend <firrtl|verilator|vcs>
    // -td  = target directory
    // -tts = seed for RNG
    chisel3.iotesters.Driver.execute(
     Array(
       // "-ll", "Info",
       "-tbn", "verilator",
       "-td", "test_run_dir",
       "-tts", "0001"),
     () => new int_expDF()) {
     c => new int_expTests(c)
    } should be(true)
  }
}

