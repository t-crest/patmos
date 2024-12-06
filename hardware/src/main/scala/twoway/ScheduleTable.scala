/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 * 
 * Schedules for the S4NOC, as described in:
 * 
 * Florian Brandner and Martin Schoeberl,
 * Static Routing in Symmetric Real-Time Network-on-Chips,
 * In Proceedings of the 20th International Conference on Real-Time
 * and Network Systems (RTNS 2012), 2012, 61-70
 * 
 * Available at:
 * https://github.com/t-crest/s4noc/tree/master/noc/vhdl/generated
 */

package s4noc_twoway

import scala.io.Source

object ScheduleTable {

  val FourNodes =
    "nel|" +
    "  nl|" +
    "   el|"

  val NineNodes =
    "nel|" +
    " nwl|" +
    "  esl|" +
    "   wsl|" +
    "     nl|" +
    "      el|" +
    "       sl|" +
    "        wl|"

  val SixTeenNodes =
    "nneel|" +
    " esl|" +
    "   neel|" +
    "    nnel|" +
    "     wnnl|" +
    "       eesl|" +
    "        nl|" +
    "         nel|" +
    "          nwl|" +
    "           nnl|" +
    "            eel|" +
    "             swl|" +
    "               el|" +
    "                sl|" +
    "                 wl|"

  val TwentyFiveNodes =
      "nneel|" +
      " sswwl|" +
      "  nl|" +
      "   nnwwl|" +
      "    eessl|" +
      "     sl|" +
      "      neel|" +
      "       nwwl|" +
      "        nnel|" +
      "         swwl|" +
      "          nnwl|" +
      "           essl|" +
      "            eesl|" +
      "              nel|" +
      "               sswl|" +
      "                el|" +
      "                 nwl|" +
      "                  nnl|" +
      "                   esl|" +
      "                    eel|" +
      "                     swl|" +
      "                      ssl|" +
      "                       wwl|" +
      "                         wl|"

  val ThirtySixNodes =
      "nnneeel|" +
      " eessl|" +
      "  swl|" +
      "    nneeel|" +
      "     wwnnnl|" +
      "      nwl|" +
      "       sswwl|" +
      "         eeessl|" +
      "          nnneel|" +
      "           wwl|" +
      "             nwwl|" +
      "              neeel|" +
      "               nnwwl|" +
      "                sl|" +
      "                 nneel|" +
      "                  ennnl|" +
      "                   wl|" +
      "                    swwl|" +
      "                     eeesl|" +
      "                      nnnwl|" +
      "                        essl|" +
      "                         neel|" +
      "                          nnel|" +
      "                           sswl|" +
      "                            nnwl|" +
      "                             eesl|" +
      "                              nnnl|" +
      "                               eeel|" +
      "                                 nel|" +
      "                                  nnl|" +
      "                                   esl|" +
      "                                    eel|" +
      "                                     ssl|" +
      "                                       nl|" +
      "                                        el|"

  val FourtyNineNodes =
      "nnneeel|" +
      " ssswwwl|" +
      "  wwl|" +
      "    nnnwwwl|" +
      "     sssl|" +
      "      eeesssl|" +
      "        nneeel|" +
      "          nnneel|" +
      "           wwwl|" +
      "            sswwwl|" +
      "             nnnl|" +
      "              ssswwl|" +
      "               eesssl|" +
      "                nel|" +
      "                 nnwwwl|" +
      "                  eeessl|" +
      "                   nnnwwl|" +
      "                    sl|" +
      "                     eessl|" +
      "                      neeel|" +
      "                       nwwwl|" +
      "                        nneel|" +
      "                          swwwl|" +
      "                           nnnel|" +
      "                            sswwl|" +
      "                             esssl|" +
      "                              nnnwl|" +
      "                               eeesl|" +
      "                                 nnwwl|" +
      "                                  eel|" +
      "                                   ssswl|" +
      "                                    nwl|" +
      "                                     neel|" +
      "                                      nwwl|" +
      "                                       nnel|" +
      "                                        swwl|" +
      "                                         nnwl|" +
      "                                          essl|" +
      "                                           eesl|" +
      "                                             eeel|" +
      "                                              sswl|" +
      "                                                nnl|" +
      "                                                 esl|" +
      "                                                   swl|" +
      "                                                    ssl|" +
      "                                                      nl|" +
      "                                                       el|" +
      "                                                        wl|"

  val SixtyFourNodes =
      "nnnneeeel|" +
      " eeesssl|" +
      "  swwwl|" +
      "   sl|" +
      "    nl|" +
      "     nnneeeel|" +
      "      wwwnnnnl|" +
      "        ssswwwl|" +
      "         wwl|" +
      "           ssswwl|" +
      "            eeeesssl|" +
      "             nnnneeel|" +
      "              sswwl|" +
      "                el|" +
      "                 nwwwl|" +
      "                  nneeeel|" +
      "                   sswl|" +
      "                     nnneeel|" +
      "                      wwnnnnl|" +
      "                       swwl|" +
      "                        sswwwl|" +
      "                          ssswl|" +
      "                            nnnwwwl|" +
      "                             eennnnl|" +
      "                              sseeeel|" +
      "                               esl|" +
      "                                  wwwl|" +
      "                                   neeeel|" +
      "                                    nnwwwl|" +
      "                                     ssl|" +
      "                                      nnnwwl|" +
      "                                       sssl|" +
      "                                        ennnnl|" +
      "                                         eesssl|" +
      "                                          swl|" +
      "                                           eeessl|" +
      "                                             nneeel|" +
      "                                              wl|" +
      "                                               nnneel|" +
      "                                                  nnnnwl|" +
      "                                                    eeeesl|" +
      "                                                      nwl|" +
      "                                                       neeel|" +
      "                                                         nneel|" +
      "                                                           nnnel|" +
      "                                                             esssl|" +
      "                                                              nnnwl|" +
      "                                                               eessl|" +
      "                                                                 nnwwl|" +
      "                                                                  eeesl|" +
      "                                                                   nnnnl|" +
      "                                                                     eeeel|" +
      "                                                                       nwwl|" +
      "                                                                        neel|" +
      "                                                                         nnel|" +
      "                                                                           nnwl|" +
      "                                                                            essl|" +
      "                                                                             nnnl|" +
      "                                                                              eesl|" +
      "                                                                                eeel|" +
      "                                                                                  nel|" +
      "                                                                                   nnl|" +
      "                                                                                    eel|"

  val EightyOneNodes =
      "nnnneeeel|" +
      " sssswwwwl|" +
      "  wwwl|" +
      "   el|" +
      "     nnnnwwwwl|" +
      "      ssssl|" +
      "        eeeessssl|" +
      "          nnneeeel|" +
      "           sl|" +
      "             nnnneeel|" +
      "              wwwwl|" +
      "                ssswwwwl|" +
      "                 nnnnl|" +
      "                  wl|" +
      "                   sssswwwl|" +
      "                    eeessssl|" +
      "                     nnel|" +
      "                       nnnwwwwl|" +
      "                        eeeesssl|" +
      "                          nnnnwwwl|" +
      "                            eeeel|" +
      "                              nneeeel|" +
      "                               sswwwwl|" +
      "                                 nnneeel|" +
      "                                  ssswwwl|" +
      "                                    nnnneel|" +
      "                                     sssswwl|" +
      "                                        nl|" +
      "                                         nnwwwwl|" +
      "                                          eeesssl|" +
      "                                           ssl|" +
      "                                            nnnwwwl|" +
      "                                              eessssl|" +
      "                                               nnnnwwl|" +
      "                                                eeeessl|" +
      "                                                   neeeel|" +
      "                                                    nwwwwl|" +
      "                                                      nneeel|" +
      "                                                        swwwwl|" +
      "                                                         nnneel|" +
      "                                                           sswwwl|" +
      "                                                            nnnnel|" +
      "                                                             ssswwl|" +
      "                                                               essssl|" +
      "                                                                nnnnwl|" +
      "                                                                  eesssl|" +
      "                                                                    nnwwwl|" +
      "                                                                     eeessl|" +
      "                                                                      nnnwwl|" +
      "                                                                        eeeesl|" +
      "                                                                          nwwwl|" +
      "                                                                           neeel|" +
      "                                                                            nnnel|" +
      "                                                                             sssswl|" +
      "                                                                               nneel|" +
      "                                                                                 swwwl|" +
      "                                                                                  nel|" +
      "                                                                                   sswwl|" +
      "                                                                                    nnnwl|" +
      "                                                                                     esssl|" +
      "                                                                                       nnwwl|" +
      "                                                                                        eessl|" +
      "                                                                                          eeesl|" +
      "                                                                                           nwl|" +
      "                                                                                            neel|" +
      "                                                                                             nwwl|" +
      "                                                                                              ssswl|" +
      "                                                                                               nnl|" +
      "                                                                                                eesl|" +
      "                                                                                                 swwl|" +
      "                                                                                                  nnwl|" +
      "                                                                                                   essl|" +
      "                                                                                                    nnnl|" +
      "                                                                                                     eeel|" +
      "                                                                                                      sswl|" +
      "                                                                                                        sssl|" +
      "                                                                                                          esl|" +
      "                                                                                                           eel|" +
      "                                                                                                            swl|" +
      "                                                                                                              wwl|"

  val OneHundredNodes =
    "nnnnneeeeel|" +
      " eeeessssl|" +
      "  swwwwl|" +
      "   ssl|" +
      "     nl|" +
      "      nnnneeeeel|" +
      "       wwwwnnnnnl|" +
      "         sssswwwwl|" +
      "          nwwl|" +
      "             sl|" +
      "              ssswwwwl|" +
      "               eeeeessssl|" +
      "                nnnnneeeel|" +
      "                 sssl|" +
      "                     nwwwwl|" +
      "                      nnneeeeel|" +
      "                        ssssl|" +
      "                         nwl|" +
      "                          nnnneeeel|" +
      "                           wwwnnnnnl|" +
      "                             sssswwwl|" +
      "                              wwwl|" +
      "                                 ssswwwl|" +
      "                                  eesl|" +
      "                                   nnnnwwwwl|" +
      "                                    eeennnnnl|" +
      "                                     ssseeeeel|" +
      "                                       esl|" +
      "                                         sswwwwl|" +
      "                                            nneeeeel|" +
      "                                             essssl|" +
      "                                              nnwwwwl|" +
      "                                               wl|" +
      "                                                nnneeeel|" +
      "                                                   nnnneeel|" +
      "                                                    wwwnnnnl|" +
      "                                                      sssswwl|" +
      "                                                       wwl|" +
      "                                                          eeessssl|" +
      "                                                           nnnwwwwl|" +
      "                                                            swl|" +
      "                                                             eennnnnl|" +
      "                                                               eeeesssl|" +
      "                                                                 sseeeeel|" +
      "                                                                   wwwwl|" +
      "                                                                    nnnnnwwl|" +
      "                                                                      swwl|" +
      "                                                                        eessssl|" +
      "                                                                         neeeeel|" +
      "                                                                          nnnwwwl|" +
      "                                                                              nneeeel|" +
      "                                                                               ennnnnl|" +
      "                                                                                 sswwwl|" +
      "                                                                                   ssswwl|" +
      "                                                                                    eeesssl|" +
      "                                                                                     nnneeel|" +
      "                                                                                        nnnneel|" +
      "                                                                                          sssswl|" +
      "                                                                                           el|" +
      "                                                                                            nnnnnwl|" +
      "                                                                                              eeeessl|" +
      "                                                                                               ssswl|" +
      "                                                                                                 nnnnwwl|" +
      "                                                                                                  eeeeesl|" +
      "                                                                                                     sswwl|" +
      "                                                                                                      neeeel|" +
      "                                                                                                       nnwwwl|" +
      "                                                                                                         nneeel|" +
      "                                                                                                          sswl|" +
      "                                                                                                           nnneel|" +
      "                                                                                                             swwwl|" +
      "                                                                                                              nnnnel|" +
      "                                                                                                                eesssl|" +
      "                                                                                                                  nnnnwl|" +
      "                                                                                                                   eeessl|" +
      "                                                                                                                      nnnwwl|" +
      "                                                                                                                       eeeesl|" +
      "                                                                                                                         nnnnnl|" +
      "                                                                                                                           eeeeel|" +
      "                                                                                                                              nwwwl|" +
      "                                                                                                                               neeel|" +
      "                                                                                                                                 nneel|" +
      "                                                                                                                                   nnnel|" +
      "                                                                                                                                     esssl|" +
      "                                                                                                                                      nnnwl|" +
      "                                                                                                                                       eessl|" +
      "                                                                                                                                         nnwwl|" +
      "                                                                                                                                          eeesl|" +
      "                                                                                                                                           nnnnl|" +
      "                                                                                                                                             eeeel|" +
      "                                                                                                                                                neel|" +
      "                                                                                                                                                 nnel|" +
      "                                                                                                                                                   nnwl|" +
      "                                                                                                                                                    essl|" +
      "                                                                                                                                                     nnnl|" +
      "                                                                                                                                                      eeel|" +
      "                                                                                                                                                        nel|" +
      "                                                                                                                                                         nnl|" +
      "                                                                                                                                                          eel|"

  def main(args: Array[String]): Unit = {
    var cnt = Source.fromFile(args(0)).getLines().length
    val lines = Source.fromFile(args(0)).getLines()
    for (l <- lines) {
      val end = if (cnt > 1) " +" else ""
      println("    \"" + l + "l|\"" + end)
      cnt -= 1
    }
  }

}
