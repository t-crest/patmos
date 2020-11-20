/* Copyright (c) 2007 Johns Hopkins University.
*  All rights reserved.
*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @author Razvan Musaloiu-E. <razvanm@cs.jhu.edu>
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 */

configuration DelugeMetadataC
{
  provides interface DelugeMetadata[uint8_t client];
  uses event void storageReady();
}

implementation
{
  components MainC;
  components DelugeMetadataP;

  DelugeMetadata = DelugeMetadataP;
  storageReady = DelugeMetadataP;
  DelugeMetadataP.Boot -> MainC;

  components new BlockReaderC(VOLUME_GOLDENIMAGE) as BlockReaderGoldenImage;
  components new BlockReaderC(VOLUME_DELUGE1) as BlockReaderDeluge1;
  components new BlockReaderC(VOLUME_DELUGE2) as BlockReaderDeluge2;
  components new BlockReaderC(VOLUME_DELUGE3) as BlockReaderDeluge3;

  DelugeMetadataP.BlockRead[VOLUME_GOLDENIMAGE] -> BlockReaderGoldenImage;
  DelugeMetadataP.BlockRead[VOLUME_DELUGE1] -> BlockReaderDeluge1;
  DelugeMetadataP.BlockRead[VOLUME_DELUGE2] -> BlockReaderDeluge2;
  DelugeMetadataP.BlockRead[VOLUME_DELUGE3] -> BlockReaderDeluge3;

  components new BlockWriterC(VOLUME_GOLDENIMAGE) as BlockWriterGoldenImage;
  components new BlockWriterC(VOLUME_DELUGE1) as BlockWriterDeluge1;
  components new BlockWriterC(VOLUME_DELUGE2) as BlockWriterDeluge2;
  components new BlockWriterC(VOLUME_DELUGE3) as BlockWriterDeluge3;

  DelugeMetadataP.BlockWrite[VOLUME_GOLDENIMAGE] -> BlockWriterGoldenImage;
  DelugeMetadataP.BlockWrite[VOLUME_DELUGE1] -> BlockWriterDeluge1;
  DelugeMetadataP.BlockWrite[VOLUME_DELUGE2] -> BlockWriterDeluge2;
  DelugeMetadataP.BlockWrite[VOLUME_DELUGE3] -> BlockWriterDeluge3;
}
