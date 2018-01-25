package dataflow

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
import arbiters._
import node._


/**
  * This Object should be initialize at the first step
  * It contains all the transformation from indecies to their module's name
  */

object Data_foo_FlowParam{

  val entry_pred = Map(
    "active" -> 0
  )


  val for_end_pred = Map(
    "m_3" -> 0
  )


  val for_end_split_pred = Map(
    "m_4" -> 0
  )


  val codeRepl_pred = Map(
    "m_1" -> 0
  )


  val m_1_brn_bb = Map(
    "codeRepl" -> 0
  )


  val m_3_brn_bb = Map(
    "for_end" -> 0
  )


  val m_4_brn_bb = Map(
    "for_end_split" -> 0
  )


  val entry_activate = Map(
    "m_0" -> 0,
    "m_1" -> 1
  )


  val codeRepl_activate = Map(
    "" -> 0,
    "m_2" -> 1,
    "m_3" -> 2
  )


  val for_end_activate = Map(
    "m_4" -> 0
  )


  val for_end_split_activate = Map(
    "m_5" -> 0,
    "m_6" -> 1,
    "m_7" -> 2,
    "m_8" -> 3,
    "m_9" -> 4,
    "m_10" -> 5,
    "m_11" -> 6
  )


  //  %sum.0.loc = alloca i32, !UID !7
  val m_0_in = Map(
  )


  //  %sum.0.reload = load i32, i32* %sum.0.loc, !UID !11
  val m_2_in = Map( 
    "m_0" -> 0
  )


  //  %mul1 = mul nsw i32 %sum.0.reload, 5, !UID !16
  val m_5_in = Map( 
    "m_2" -> 0
  )


  //  %arrayidx2 = getelementptr inbounds i32, i32* %out1, i64 0, !UID !17
  val m_6_in = Map( 
    "data_1" -> 0
  )


  //  store i32 %mul1, i32* %arrayidx2, align 4, !UID !18
  val m_7_in = Map( 
    "m_5" -> 0,
    "m_6" -> 0
  )


  //  %mul3 = mul nsw i32 %sum.0.reload, 8, !UID !19
  val m_8_in = Map( 
    "m_2" -> 1
  )


  //  %arrayidx4 = getelementptr inbounds i32, i32* %out2, i64 0, !UID !20
  val m_9_in = Map( 
    "data_2" -> 0
  )


  //  store i32 %mul3, i32* %arrayidx4, align 4, !UID !21
  val m_10_in = Map( 
    "m_8" -> 0,
    "m_9" -> 0
  )


  //  ret void, !UID !22, !BB_UID !23
  val m_11_in = Map(
  )


}





  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */


abstract class fooDFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
    val data_0 = Flipped(Decoupled(new DataBundle))
    val data_1 = Flipped(Decoupled(new DataBundle))
    val data_2 = Flipped(Decoupled(new DataBundle))
    val data_3 = Flipped(Decoupled(new DataBundle))
    val data_4 = Flipped(Decoupled(new DataBundle))
    val pred = Decoupled(new Bool())
    val start = Input(new Bool())
  })
}

class fooDF(implicit p: Parameters) extends fooDFIO()(p) {




  /* ================================================================== *
   *                   PRINTING MODULE DEFINITION                       *
   * ================================================================== */


