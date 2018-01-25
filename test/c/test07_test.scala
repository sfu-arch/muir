package dataflow

import chisel3._
import chisel3.util._
import chisel3.Module
import chisel3.testers._
import chisel3.iotesters._
import org.scalatest.{FlatSpec, Matchers}
import muxes._
import config._
import control._
import util._
import interfaces._
import regfile._
import memory._
import stack._
import arbiters._
import loop._
import accel._
import node._


class test07CacheWrapper()(implicit p: Parameters) extends test07DF()(p)
  with CacheParams {

  // Instantiate the AXI Cache
  val cache = Module(new Cache)
  cache.io.cpu.req <> CacheMem.io.CacheReq
  CacheMem.io.CacheResp <> cache.io.cpu.resp
  cache.io.cpu.abort := false.B
  // Instantiate a memory model with AXI slave interface for cache
  val memModel = Module(new NastiMemSlave)
  memModel.io.nasti <> cache.io.nasti

}

class test07Test01(c: test07CacheWrapper) extends PeekPokeTester(c) {


  /**
  *  test07DF interface:
  *
   *    val pred = Decoupled(new Bool())
   *    val start = Input(new Bool())
   *    val result = Decoupled(new DataBundle)
   */


  // Initializing the signals

  poke(c.io.entry.bits.control, false.B)
  poke(c.io.entry.valid, false.B)

  poke(c.io.result.ready, false.B)

/**
   *
   * @todo Add your test cases here
   *
   * The test harness API allows 4 interactions with the DUT:
   *  1. To set the DUT'S inputs: poke
   *  2. To look at the DUT'S outputs: peek
   *  3. To test one of the DUT's outputs: expect
   *  4. To advance the clock of the DUT: step
   *
   * Conditions:
   *  1. while(peek(c.io.XXX) == UInt(0))
   *  2. for(i <- 1 to 10)
   *  3. for{ i <- 1 to 10
   *          j <- 1 to 10
   *        }
   *
   * Print Statement:
   *    println(s"Waited $count cycles on gcd inputs $i, $j, giving up")
   *
   */

  // Example to just increment clock 150 times
  step(1)
  var time = 1  //Cycle counter
  while (time < 150) {
   time += 1
   step(1)
   println(s"Cycle: $time")
  }

}

class test07Tester extends FlatSpec with Matchers {
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  it should "Check that test07 works correctly." in {
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
     () => new test07CacheWrapper()) {
     c => new test07Test01(c)
    } should be(true)
  }
}

