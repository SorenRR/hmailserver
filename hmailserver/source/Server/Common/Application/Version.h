#pragma once

#define HMAILSERVER_VERSION "5.6.9"
#define HMAILSERVER_BUILD "2644.97 (x86)"

/*
[list=1]
[*]Supports Sub OnHELO(oClient) event, [url=https://github.com/hmailserver/hmailserver/issues/153]issue #153[/url]
[*]Fixed Incorrect DEBUG logging for event 'OnDeliverMessage', [url=https://github.com/hmailserver/hmailserver/issues/181]issue #181[/url]
[*]Include HTMLBody into IMAP TEXT search, [url=https://github.com/hmailserver/hmailserver/pull/193]pull #193[/url]
[*]Fixed implicit conversion: "int" to "unsigned char" [url=https://github.com/hmailserver/hmailserver/pull/204]pull #204[/url]
[*]Faulty: SMTP 'Disconnect client after too many invalid commands'  pull [url=https://github.com/hmailserver/hmailserver/issues/160]issue #160[/url]
[*]SMTP server error "550 Unsupported ESMTP extension" on MAIL FROM:... AUTH=<> [with fix] [url=https://github.com/hmailserver/hmailserver/issues/164]issue #164[/url]
[*]Removed warning if backup was more than 1,5GB and 15GB limit. There's no longer a recommended max-size - the time will vary with the installation size. [url=https://github.com/hmailserver/hmailserver/issues/69]issue #69[/url]
[*]Speed up 'update hm_messages set messageflags' [url=https://github.com/hmailserver/hmailserver/issues/221]issue  #221[/url]
[*]Add [url=https://www.hmailserver.com/forum/viewtopic.php?p=177487#p177487]Return-Path header[/url] as topmost header before sending the message to SA (+ delete Return-Path header after the SA check completes) [url=https://github.com/hmailserver/hmailserver/issues/116]issue #116[/url]
[*]Experimental eventhandler OnClientLogon(oClient), New ClientInfo property oClient.Authenticated (Boolean)
[*]Handling of long UIDL response lists was too slow. [url=https://github.com/hmailserver/hmailserver/issues/93]issue #93[/url]
[*]When calling SpamAssassin and there was a connection failure, sometimes temporary files were left behind [url=https://github.com/hmailserver/hmailserver/issues/100]issue #100[/url]
[*]SURBL detection properly fails to detect url's ending with a query string [url=https://github.com/hmailserver/hmailserver/issues/108]issue  #108[/url]
[*]If a route is set up, but the recipient does not match an address in the route address list, the domain catch-all should be used if specified. [url=https://github.com/hmailserver/hmailserver/issues/74]issue #74[/url]
[*]Fix ExternalFetcher DELE when no RETR, pull [url=https://github.com/hmailserver/hmailserver/pull/254]pull #254[/url]
[*]SMTP multiply max message size with 1024 [url=https://github.com/hmailserver/hmailserver/issues/267]issue #267[/url]
[*]Add email address variable to SignatureAdder.cpp [url=https://github.com/hmailserver/hmailserver/pull/256]pull #265[/url]
[*]DKIM on acccount-rule 'reply' not applied #172 [url=https://github.com/hmailserver/hmailserver/issues/172]issue #172[/url]
[*]preserve RewriteEnvelopeFromWhenForwarding setting [url=https://www.hmailserver.com/forum/viewtopic.php?f=7&t=34026]when forwarding from account rule[/url]
[*]The logical flow should be to disregard "Require SMTP authentication" if "Allow deliveries from" is unselected [url=https://github.com/hmailserver/hmailserver/issues/287]issue #287[/url]
[*]Add ability to DKIM sign NDR messages (forwarded to external) [url=https://github.com/hmailserver/hmailserver/pull/301]pull #301[/url]
[*]Use custom daemonaddressdomain from INI [url=https://github.com/hmailserver/hmailserver/pull/301]pull #301[/url]
[*]Fix SURBL regex [url=https://github.com/hmailserver/hmailserver/pull/320]pull #320[/url]
[*]Ignore SpamTestSPF and SpamTestHeloHost when send thru local IP Address, see [url=https://www.hmailserver.com/forum/viewtopic.php?f=10&t=34865]this[/url] forum topic 
[*]SPF test to include "HELO/EHLO host" used by DNS macros [url=https://github.com/hmailserver/hmailserver/pull/353]pull 353[/url]
[*]Fix 'Unencrypted Cleartext Login' vulnerability detection [url=https://github.com/hmailserver/hmailserver/pull/342]pull 242[/url]
[*]Add SMTP MAIL FROM header: X-hMailServer-Envelope-From. Add following to SpamAssassin local.cf: envelope_sender_header X-hMailServer-Envelope-From
[*]New ClientInfo properties oClient.ESMTP(boolean), oClient.TLS(boolean) and oClient.Passwd(string)
[*]Added "UseDNSCache" option to [Settings]. 1 = Use cache in DNS Client service. 0 = Bypass cache in DNS Client service.
[*]"UseDNSCache" control temporary fix; Messages bouncing immediately due to no mail servers due to DNS issue "ExternalDelivery.cpp"
[*]Anti-spam; Maximum message to scan (KB) is now limited to 5 MB if entered value = 0, otherwise whatever is entered into GUI.
[*]AWStats::LogDeliveryFailure was called twice for each message in OnDeliveryFailed
[*]oMessage object was empty when called from OnDeliveryFailed in SMTPDeliverer.cpp
[*]Fix Auto Submitted header for auto-generated messages
[*]Sub OnClientLogon(oClient) only show password for failed login
[*]Added: "DNSServer" INI setting. Single (local) DNS server IPv4 addresses to use within hMailServer instead of default system DNS.
[*]Added: #251 External Fetch - First try (X-)Envelope-To and X-RCPT-TO headers and ExternalFetcher DELE when no RETR
[*]Added: #337 Set the JScript version to use
[*]Added: #383 DKIM signature for domain aliases
[*]Added: New ClientInfo properties oClient.EncryptedConnection (Boolean), oClient.CipherVersion, oClient.CipherName and oClient.CipherBits
[*]Added: DomainMembers DistributionList mode [url=https://github.com/hmailserver/hmailserver/pull/392]pull #392[/url]
[*]Added: oClient.SessionID "The SessionID for TCP/IP connection."
[*]Added: X-hMailServer-Envelope-From header, grouping off all X-hMailServer-* headers, For consistency equalize Return-Path format used within hMailServer, eg: Return-Path: <return-route-addr> (always use angle brackets)
[*]Added: TLS server cipher preference support and support for prioritizing ChaCha20Poly1305 [url=https://github.com/hmailserver/hmailserver/pull/379]pull 379[/url]
[*]Added: rDNS/PTR AntiSpam Check [url=https://github.com/hmailserver/hmailserver/pull/413]pull #413[/url]
[*]Added: Prefer IPv6 over IPv4 [url=https://github.com/hmailserver/hmailserver/pull/415]pull #415[/url]
[*]Fix: SpamTestHeloHost IPv6 Fix [url=https://github.com/hmailserver/hmailserver/pull/416]pull #416[/url]
{*]Mod: Changed ClamAV Scan from STREAM to INSTREAM.
[*]Added: MIME Recipient Headers as configurable comma delimited string [url=https://github.com/hmailserver/hmailserver/pull/428]pull #428[/url]
[*]Fix: AUTH PLAIN followed by base64 encoded username and password (on same line) in log exposed account password https://github.com/hmailserver/hmailserver/pull/437
[*]Fix: After an AUTH command has been successfully completed, no more AUTH commands may be issued in the same session. https://github.com/hmailserver/hmailserver/pull/438
[*]Fix: Typo in error message 5507 and 5508 in SpamTestSpamAssassin.cpp
[*]Fix: HM5157 SpamAssassinClient::OnReadError, The WinSock error code is 2. [url=https://github.com/hmailserver/hmailserver/issues/167]issue #167[/url]
[*]Fix: TCP/IP Ports 'Default' button in GUI did not add the standard port SMTP 587 (this port is standard added by SQL script during installation) #pull 441
[*]Fix: RMSPF version 1.12. Changed MAX SPF lookups from 20 to 10 as per RFC 7208.
[*]Removed: Collection of statistics, since it's no longer being used. issue #435
[*]Fix: If MSSQL OLE DB Provider 18 or later is installed, prefer that one, since it supports TLS1.1/1.2 which older providers do not. [url=https://github.com/hmailserver/hmailserver/issues/186]issue 186[/url]
[*]Fix: Rework of issue #167 to check for high CPU problem inside TCPConnection::AsyncReadCompleted.
[*]Added: %MACRO_ORIGINAL_HEADER% macro expansion for 'Set header value' rules
[*]Fix: S/MIME Signed Message Failing Signature Validation due to X-hMailServer-Envelope-From header position, effectively this means ungrouping of the X-hMailServer-* headers
[*]Fix: Error logged if email address length exceeds 254 characters [url=https://github.com/hmailserver/hmailserver/issues/393]issue #393[/url]
[*]Fix: DKIM Signing Not Performed On Messages > 10 MB [url=https://github.com/hmailserver/hmailserver/issues/387]issue #387[/url]
[*]Fix: DKIM (header) verification fails when an email is received with no subject issue #245
[*]Added: DKIM signing support for CSA-Compliant Mail Headers, eg: X-CSA-Complaints and List-Unsubscribe-Post headers.
[*]Fix: Prefer IPv6 over IPv4 didn't do anything, added the missing part (sorting of found IP Addresses)
[*]Update: Reverted a workaround to Thunderbird bug https://bugzilla.mozilla.org/show_bug.cgi?id=460443
[*]Added: Received-SPF: diagnostic header controlled with INI setting "AddReceivedSPFHeader" (skip for authenticated client connections or localhost!)
[*]Added: (envelope-from <user@domain.com>), for <user@domain.com> to Received: header (if single recipient!)
[*]Moving "Return-Path" header handling to SMTPMessageHeaderCreator.
[*]Scripting "Received" header to handle HELO/EHLO/ESMTP/SMTP in header.
[*]Update: Removed X-Envelope headers.
[*]Update: Distinguish more clearly between helo/mailfrom SPF results/checks
[*]Update: Remove .NET 2.0 requirement during install.
[*]Update hm_smtp_antivirus.php issue #471
[*]Update: Adjusted the number of triggers of "invalid command" in SMTPConnection.cpp
[*]Fix: RFC822.SIZE https://github.com/hmailserver/hmailserver/issues/476
[*]Added: OnRecipientUnknown(oClient, oMessage) and OnTooManyInvalidCommands(oClient, oMessage)
[*]Update: Done som MIME Folding changes and changed a few error messages.
[*]Fix: hMailServer AUTH PLAIN in SMTP fails when authzid is supplied
[*]Fix: fileutilities.cpp - bytes->GetSize() crashes when bytes = std::shared_ptr<ByteBuffer> empty;
[*]Fix: Minor bugfix where the Received-SPF diagnostic header gave inaccurate results when receiving mail through a (trusted/internal) relay
[*]Added: IPv6 Support for BLCheck [url=https://github.com/hmailserver/hmailserver/pull/487]pull 487[/url]
[*]Update: Replacement for the old SA "winsock error 2" workaround/fix and eliminates (all) EOF errors/warnings (and forced disconnects, possible related to HM5136?) altogether
[*]Update: OpenSSL 3.0.14
[*]Fix: Improved AUTH PLAIN base64 encoded username and password masking, retain client command format for troubleshooting purposes
[*]Fix: SURBL modification to check full URI's and trimmed down URI's
[*]Fix: Strip possible spaces in DKIM 'p' parameter, there shouldn't be any spaces but it's a common mistake so we act lenient and strip any spaces found
[*]Update: remove password blocking.
[/list]
*/