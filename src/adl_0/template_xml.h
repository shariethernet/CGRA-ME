/*******************************************************************************
 * CGRA-ME Software End-User License Agreement
 *
 * The software programs comprising "CGRA-ME" and the documentation provided
 * with them are copyright by its authors S. Chin, K. Niu, N. Sakamoto, J. Zhao,
 * A. Rui, S. Yin, A. Mertens, J. Anderson, and the University of Toronto. Users
 * agree to not redistribute the software, in source or binary form, to other
 * persons or other institutions. Users may modify or use the source code for
 * other non-commercial, not-for-profit research endeavours, provided that all
 * copyright attribution on the source code is retained, and the original or
 * modified source code is not redistributed, in whole or in part, or included
 * in or with any commercial product, except by written agreement with the
 * authors, and full and complete attribution for use of the code is given in
 * any resulting publications.
 *
 * Only non-commercial, not-for-profit use of this software is permitted. No
 * part of this software may be incorporated into a commercial product without
 * the written consent of the authors. The software may not be used for the
 * design of a commercial electronic product without the written consent of the
 * authors. The use of this software to assist in the development of new
 * commercial CGRA architectures or commercial soft processor architectures is
 * also prohibited without the written consent of the authors.
 *
 * This software is provided "as is" with no warranties or guarantees of
 * support.
 *
 * This Agreement shall be governed by the laws of Province of Ontario, Canada.
 *
 * Please contact Prof. Anderson if you are interested in commercial use of the
 * CGRA-ME framework.
 ******************************************************************************/

#ifndef TEMPLATE_XML_H_INCLUDED
#define TEMPLATE_XML_H_INCLUDED

namespace adl0 {

const char * template_xml = R"*************(
<?xml version="1.0"?>
<templates>
  <template pattern="mesh" attributes="out-north out-east out-west out-south
    out-northeast out-northwest out-southeast out-southwest
    in-north in-east in-west in-south
    in-northeast in-northwest in-southeast in-southwest
    edge-out edge-in edge io">
    <copy element="temp-pattern" pattern="interior">
      <attribute name="col-range" value="1 (- (cols) 2)" overwrite="false"/>
      <attribute name="row-range" value="1 (- (rows) 2)" overwrite="false"/>
    </copy>
    <copy element="temp-pattern" pattern="exterior">
      <attribute name="only-edge" value="(attribute edge)" overwrite="false"/>
    </copy>
    <append element="temp-pattern">
      <attribute name="col-range" value="1 (- (cols) 2)"/>
      <attribute name="row-range" value="1 (- (rows) 2)"/>
      <children>
        <block>
          <!-- a block can be referred with relative position using "(rel dx dy)"  -->
          <connection from-prefix="(rel  0 -1)" from="(attribute out-east)" to-prefix="(rel 0 0)" to="(attribute in-west)" if="(not (or (on-edge west) (on-edge northwest) (on-edge southwest)))"/>
          <connection from-prefix="(rel  0  1)"  from="(attribute out-west)" to-prefix="(rel 0 0)" to="(attribute in-east)" if="(not (or (on-edge east) (on-edge northeast) (on-edge southeast)))"/>
          <connection from-prefix="(rel  1  0)"  from="(attribute out-north)" to-prefix="(rel 0 0)" to="(attribute in-south)" if="(not (or (on-edge south) (on-edge southeast) (on-edge southwest)))"/>
          <connection from-prefix="(rel -1  0)" from="(attribute out-south)" to-prefix="(rel 0 0)" to="(attribute in-north)" if="(not (or (on-edge north) (on-edge northeast) (on-edge northwest)))"/>
          <connection from-prefix="(rel -1 -1)" from="(attribute out-southeast)" to-prefix="(rel 0 0)" to="(attribute in-northwest)" if="(not (or (on-edge west) (on-edge southwest) (on-edge north) (on-edge northeast) (on-edge northwest)))"/>
          <connection from-prefix="(rel  1 -1)" from="(attribute out-northeast)" to-prefix="(rel 0 0)" to="(attribute in-southwest)" if="(not (or (on-edge west) (on-edge northwest) (on-edge south) (on-edge southeast) (on-edge southwest)))"/>
          <connection from-prefix="(rel -1  1)"  from="(attribute out-southwest)" to-prefix="(rel 0 0)" to="(attribute in-northeast)" if="(not (or (on-edge north) (on-edge northwest) (on-edge east) (on-edge northeast) (on-edge southeast)))"/>
          <connection from-prefix="(rel  1  1)"  from="(attribute out-northwest)" to-prefix="(rel 0 0)" to="(attribute in-southeast)" if="(not (or (on-edge east) (on-edge northeast) (on-edge south) (on-edge southeast) (on-edge southwest)))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="north"/>
      <attribute name="if" value="(elem (attribute io) manual)"/>
      <children>
        <block>
          <connection from-prefix="(rel 1 0)" from="(attribute out-north)" to-prefix="(rel 0 0)" to="(attribute edge-in)"/>
          <connection from-prefix="(rel 0 0)" from="(attribute edge-out)" to-prefix="(rel 1 0)" to="(attribute in-north)"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="south"/>
      <attribute name="if" value="(elem (attribute io) manual)"/>
      <children>
        <block>
          <connection from-prefix="(rel -1 0)" from="(attribute out-south)" to-prefix="(rel 0 0)" to="(attribute edge-in)"/>
          <connection from-prefix="(rel  0 0)" from="(attribute edge-out)" to-prefix="(rel -1 0)" to="(attribute in-south)"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="west"/>
      <attribute name="if" value="(elem (attribute io) manual)"/>
      <children>
        <block>
          <connection from-prefix="(rel 0 1)" from="(attribute out-west)" to-prefix="(rel 0 0)" to="(attribute edge-in)"/>
          <connection from-prefix="(rel 0 0)" from="(attribute edge-out)" to-prefix="(rel 0 1)" to="(attribute in-west)"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="east"/>
      <attribute name="if" value="(elem (attribute io) manual)"/>
      <children>
        <block>
          <connection from-prefix="(rel 0 -1)" from="(attribute out-east)" to-prefix="(rel 0 0)" to="(attribute edge-in)"/>
          <connection from-prefix="(rel 0  0)" from="(attribute edge-out)"  to-prefix="(rel 0 -1)" to="(attribute in-east)"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="north"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="north"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel 1 0)" from="(attribute out-(target-port))" to-prefix="io_(rel 0 0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel 0 0)" from="_(target-port).out" to-prefix="(rel 1 0)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="south"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="south"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel -1 0)" from="(attribute out-(target-port))" to-prefix="io_(rel  0 0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0 0)" from="_(target-port).out" to-prefix="(rel -1 0)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="west"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="west"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  0  1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  0  1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="east"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="east"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  0 -1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  0 -1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
  </template>
  <template pattern="diagonal" attributes="out-north out-east out-west out-south
    out-northeast out-northwest out-southeast out-southwest
    in-north in-east in-west in-south
    in-northeast in-northwest in-southeast in-southwest
    io">
    <copy element="temp-pattern" pattern="interior">
      <attribute name="col-range" value="1 (- (cols) 2)" overwrite="false"/>
      <attribute name="row-range" value="1 (- (rows) 2)" overwrite="false"/>
    </copy>
    <copy element="temp-pattern" pattern="exterior">
      <attribute name="only-edge" value="(attribute edge)" overwrite="false"/>
    </copy>
    <append element="temp-pattern">
      <attribute name="col-range" value="1 (- (cols) 2)"/>
      <attribute name="row-range" value="1 (- (rows) 2)"/>
      <children>
        <block>
          <!-- a block can be referred with relative position using "(rel dx dy)"  -->
          <connection from-prefix="(rel  0 -1)" from="(attribute out-east)" to-prefix="(rel 0 0)" to="(attribute in-west)" if="(not (or (on-edge west) (on-edge northwest) (on-edge southwest)))"/>
          <connection from-prefix="(rel  0  1)"  from="(attribute out-west)" to-prefix="(rel 0 0)" to="(attribute in-east)" if="(not (or (on-edge east) (on-edge northeast) (on-edge southeast)))"/>
          <connection from-prefix="(rel  1  0)"  from="(attribute out-north)" to-prefix="(rel 0 0)" to="(attribute in-south)" if="(not (or (on-edge south) (on-edge southeast) (on-edge southwest)))"/>
          <connection from-prefix="(rel -1  0)" from="(attribute out-south)" to-prefix="(rel 0 0)" to="(attribute in-north)" if="(not (or (on-edge north) (on-edge northeast) (on-edge northwest)))"/>
          <connection from-prefix="(rel -1 -1)" from="(attribute out-southeast)" to-prefix="(rel 0 0)" to="(attribute in-northwest)" if="(not (or (on-edge west) (on-edge southwest) (on-edge north) (on-edge northeast) (on-edge northwest)))"/>
          <connection from-prefix="(rel  1 -1)" from="(attribute out-northeast)" to-prefix="(rel 0 0)" to="(attribute in-southwest)" if="(not (or (on-edge west) (on-edge northwest) (on-edge south) (on-edge southeast) (on-edge southwest)"/>
          <connection from-prefix="(rel -1  1)"  from="(attribute out-southwest)" to-prefix="(rel 0 0)" to="(attribute in-northeast)" if="(not (or (on-edge north) (on-edge northwest) (on-edge east) (on-edge northeast) (on-edge southeast)))"/>
          <connection from-prefix="(rel  1  1)"  from="(attribute out-northwest)" to-prefix="(rel 0 0)" to="(attribute in-southeast)" if="(not (or (on-edge east) (on-edge northeast) (on-edge south) (on-edge southeast) (on-edge southwest)))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="north"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="north"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel 1 0)" from="(attribute out-(target-port))" to-prefix="io_(rel 0 0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel 0 0)" from="_(target-port).out" to-prefix="(rel 1 0)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="south"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="south"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel -1 0)" from="(attribute out-(target-port))" to-prefix="io_(rel  0 0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0 0)" from="_(target-port).out" to-prefix="(rel -1 0)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="west"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="west"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  0  1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  0  1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="east"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="east"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  0 -1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  0 -1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="west northwest north"/>
      <attribute name="col-range" value="0 (- (cols) 2)"/>
      <attribute name="row-range" value="0 (- (rows) 2)"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="northwest"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  1  1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  1  1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="north northeast east"/>
      <attribute name="col-range" value="1 (- (cols) 1)"/>
      <attribute name="row-range" value="0 (- (rows) 2)"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="northeast"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel  1 -1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel  1 -1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="east southeast south"/>
      <attribute name="col-range" value="1 (- (cols) 1)"/>
      <attribute name="row-range" value="1 (- (rows) 1)"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="southeast"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel -1 -1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel -1 -1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
    <append element="temp-pattern">
      <attribute name="only-edge" value="south southwest west"/>
      <attribute name="col-range" value="0 (- (cols) 2)"/>
      <attribute name="row-range" value="1 (- (rows) 1)"/>
      <attribute name="if" value="(elem (attribute io) every-side-port)"/>
      <children>
        <block>
          <definition variable="target-port" be="southwest"/>
          <inout name="bidir_(rel 0 0)_(target-port)"/>
          <inst name="io_(rel 0 0)_(target-port)" module="io"/>
          <connection from="this.bidir_(rel 0 0)_(target-port)" to="io_(rel 0 0)_(target-port).bidir"/>
          <connection from-prefix="(rel -1  1)" from="(attribute out-(target-port))" to-prefix="io_(rel  0  0)" to="_(target-port).in"/>
          <connection from-prefix="io_(rel  0  0)" from="_(target-port).out" to-prefix="(rel -1  1)" to="(attribute in-(target-port))"/>
        </block>
      </children>
    </append>
  </template>
</templates>
)*************";

}

#endif
