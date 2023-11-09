// Copyright (c) 2010 Martin Knafve / hMailServer.com.  
// http://www.hmailserver.com

#include "stdafx.h"
#include "TCPServer.h"
#include "DNSResolver.h"
#include <iphlpapi.h>
#include <windns.h>
#include <boost/asio.hpp>

#include "HostNameAndIpAddress.h"

using boost::asio::ip::tcp;

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

namespace HM
{
   struct structMailServer
   {
      long lPreference;
      String sHostName;
   };

   DNSResolver::DNSResolver()
   {

   }

   DNSResolver::~DNSResolver()
   {

   }
 
   void 
   _FreeDNSRecord(PDNS_RECORD pRecord)
   {
      if (!pRecord) 
         return;

      DNS_FREE_TYPE freetype = DnsFreeRecordListDeep;
      DnsRecordListFree(pRecord, freetype);
   }

   //---------------------------------------------------------------------------()
   // DESCRIPTION:
   // Determines whether the result of a DnsQuery call is an error or not.
   //---------------------------------------------------------------------------()
   bool
   DNSResolver::IsDNSError_(int iErrorMessage)
   {
      // Assume non-fatal if we aren't sure it's not.
      switch (iErrorMessage)
      {
      case DNS_ERROR_RCODE_NAME_ERROR: // Domain doesn't exist
         return false;
      case ERROR_INVALID_NAME:
         return false;
      case DNS_INFO_NO_RECORDS: // No records were found for the host. Not an error.
         return false;
      case DNS_ERROR_NO_DNS_SERVERS: // No DNS servers found.
         return true;
      }

      return true;
   }


   //---------------------------------------------------------------------------()
   // DESCRIPTION:
   // Determines whether the result of a WSA-function is an error or not.
   //---------------------------------------------------------------------------()
   bool
   DNSResolver::IsWSAError_(int iErrorMessage)
   {
      // Assume non-fatal if we aren't sure it's not.
      switch (iErrorMessage)
      {
      case WSAHOST_NOT_FOUND: // Domain doesn't exist
         return false;
      case WSANO_DATA: // Domain doesn't exist
         return false;
      }

      return true;
   }

   bool
   DNSResolver::Resolve_(const String &sSearchFor, std::vector<String> &vecFoundNames, WORD wType, int iRecursion)
   {
      USES_CONVERSION;

      if (iRecursion > 10)
      {
         String sMessage;
         sMessage.Format(_T("Too many recursions during query. Query: %s, Type: %d."), sSearchFor.c_str(), wType);
         ErrorManager::Instance()->ReportError(ErrorManager::Low, 4401, "DNSResolver::Resolve_", sMessage);

         return false;
      }

      PDNS_RECORD pDnsRecords = NULL;
      PIP4_ARRAY pSrvList = NULL;

      DWORD fOptions;
      fOptions = DNS_QUERY_STANDARD;

      if (!IniFileSettings::Instance()->GetUseDNSCache())
      {
         fOptions += DNS_QUERY_BYPASS_CACHE;
      }

      AnsiString sCustomDNS;
      sCustomDNS = IniFileSettings::Instance()->GetDNSServer().Trim();
      if (!sCustomDNS.IsEmpty())
      {
         pSrvList = (PIP4_ARRAY)malloc(sizeof(IP4_ARRAY));
         if (!pSrvList){

            String sMessage;
            sMessage.Format(_T("Unable to allocate memory for DNS server list. Query: %s, Type: %d."), sSearchFor, wType);
            ErrorManager::Instance()->ReportError(ErrorManager::Low, 4401, "DNSResolver::_Resolve", sMessage);

            return false;
         }

         // Custom DNSServer IPv4 address
         pSrvList->AddrCount = 1;
         pSrvList->AddrArray[0] = inet_addr(sCustomDNS.c_str());

         if (pSrvList->AddrArray[0] == INADDR_NONE) {

            String sMessage;
            sMessage.Format(_T("Invalid DNS server IP address. DNS Server: %hs."), sCustomDNS.c_str());
            ErrorManager::Instance()->ReportError(ErrorManager::Low, 4401, "DNSResolver::_Resolve", sMessage);
           
            // fallback to the system dns servers
            pSrvList = NULL;
         }
         else
         {
            // We need this if not using system dns servers
            if (fOptions != DNS_QUERY_BYPASS_CACHE)
               fOptions += DNS_QUERY_BYPASS_CACHE;
         }
      }
      
      DNS_STATUS nDnsStatus = DnsQuery(sSearchFor, wType, fOptions, pSrvList, &pDnsRecords, NULL);

      PDNS_RECORD pDnsRecordsToDelete = pDnsRecords;

      if (nDnsStatus != 0)
      {
         if (pDnsRecordsToDelete)
            _FreeDNSRecord(pDnsRecordsToDelete);

         bool bDNSError = IsDNSError_(nDnsStatus);

         if (bDNSError)
         {
            String sMessage;
            sMessage.Format(_T("DNS - Query failure. Treating as temporary failure. Query: %s, Type: %d, DnsQuery return value: %d."), sSearchFor.c_str(), wType, nDnsStatus);
            LOG_TCPIP(sMessage);
            return false;
         }

         return true;
      }

      switch (wType)
      {
         case DNS_TYPE_CNAME:
         {
            String sDomainName = pDnsRecords->Data.CNAME.pNameHost;
            if (!Resolve_(sDomainName, vecFoundNames, wType, iRecursion+1))
               return false;

            break;
         }
         case DNS_TYPE_MX: 
            {
               if (pDnsRecords->wType == DNS_TYPE_CNAME)
               {
                  // we received a CNAME response so we need to recurse over that.
                  String sDomainName = pDnsRecords->Data.CNAME.pNameHost;
                  if (!Resolve_(sDomainName, vecFoundNames, DNS_TYPE_MX, iRecursion+1))
                     return false;
               }            
               else if (pDnsRecords->wType == DNS_TYPE_MX)
               {
                  std::vector<structMailServer> vecMailServersUnsorted;

                  do 
                  {
                     String sName = pDnsRecords->pName;
                     bool bNameMatches = (sName.CompareNoCase(sSearchFor) == 0);

                     if (pDnsRecords->Flags.S.Section == DNSREC_ANSWER && bNameMatches) 
                     {
                        structMailServer structServer; 
                        structServer.sHostName = pDnsRecords->Data.MX.pNameExchange;
                        structServer.lPreference = pDnsRecords->Data.MX.wPreference;

                        vecMailServersUnsorted.push_back(structServer);
                     }
                     pDnsRecords = pDnsRecords->pNext;
                  } while (pDnsRecords);

                  // Add the servers in sorted order.
                  while (vecMailServersUnsorted.size() > 0)
                  {
                     // Find lowest preference.
                     auto iterServer = vecMailServersUnsorted.begin();
                     long iCurLowest = -1;
                     while (iterServer != vecMailServersUnsorted.end())
                     {
                        if (iCurLowest == -1 || (*iterServer).lPreference < iCurLowest)
                           iCurLowest = (*iterServer).lPreference;

                        iterServer++;
                     }

                     // Add lowest to sorted list and remove from unsorted.
                     iterServer = vecMailServersUnsorted.begin();
                     while (iterServer != vecMailServersUnsorted.end())
                     {
                        if ((*iterServer).lPreference == iCurLowest)
                        {
                           vecFoundNames.push_back((*iterServer).sHostName);
                           iterServer = vecMailServersUnsorted.erase(iterServer);
                        }
                        else
                           iterServer++;
                     }
                  }
               }
            }
         break;
      case DNS_TYPE_TEXT: 
         {
            if (pDnsRecords->wType == DNS_TYPE_CNAME)
            {
               // we received a CNAME response so we need to recurse over that.
               String sDomainName = pDnsRecords->Data.CNAME.pNameHost;
               if (!Resolve_(sDomainName, vecFoundNames, DNS_TYPE_TEXT, iRecursion+1))
                  return false;
            }   
            else if (pDnsRecords->wType == DNS_TYPE_TEXT)
            {
               AnsiString retVal;

               for (u_int i = 0; i < pDnsRecords->Data.TXT.dwStringCount; i++) 
               {
                  retVal += pDnsRecords->Data.TXT.pStringArray[i];
               }
               
               vecFoundNames.push_back(retVal);
            }
            break;
         }
      // JDR: Added to perform PTR lookups.
      case DNS_TYPE_PTR: 
         {
            if (pDnsRecords->wType == DNS_TYPE_CNAME)
            {
               // we received a CNAME response so we need to recurse over that.
               String sDomainName = pDnsRecords->Data.CNAME.pNameHost;
               if (!Resolve_(sDomainName, vecFoundNames, DNS_TYPE_PTR, iRecursion+1))
                  return false;
            }   
            else if (pDnsRecords->wType == DNS_TYPE_PTR)
            {
               AnsiString retVal;
               retVal = pDnsRecords->Data.PTR.pNameHost;
               vecFoundNames.push_back(retVal);
            }
            break;
         }
         default:
            {
               ErrorManager::Instance()->ReportError(ErrorManager::Medium, 5036, "DNSResolver::Resolve_", "Querying unknown wType.");
            }
            
            break;
      }

      _FreeDNSRecord(pDnsRecordsToDelete);
      pDnsRecordsToDelete = 0;

      if (pSrvList != NULL)
         free(pSrvList);

      return true;
   }


   //---------------------------------------------------------------------------()
   // DESCRIPTION:
   // Do a DNS A/AAAA lookup.
   // 
   // Code is platform independent.
   //---------------------------------------------------------------------------()
   bool 
   DNSResolver::GetARecords(const String &sDomain, std::vector<String> &saFoundNames)
   {
      if (sDomain.IsEmpty())
      {
         ErrorManager::Instance()->ReportError(ErrorManager::Medium, 5516, "DNSResolver::GetARecords", "Attempted DNS lookup for empty host name.");
         return false;
      }


      // Do a DNS/A lookup. This may result in a AAAA result, if IPV6 is installed in the system.
      boost::asio::io_service io_service;

      // Get a list of endpoints corresponding to the server name.
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(AnsiString(sDomain), "25", tcp::resolver::query::numeric_service);

      boost::system::error_code errorCode;
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, errorCode);
      
      if (errorCode)
      {
         bool bDNSError = IsWSAError_(errorCode.value());

         if (bDNSError)
         {
            String sMessage;
            sMessage.Format(_T("DNS query failure. Query: %s, Type: A/AAAA, DnsQuery return value: %d. Message: %s"), sDomain.c_str(), errorCode.value(), String(errorCode.message()));
            LOG_TCPIP(sMessage);
            return false;
         }

         return true;
      }

      std::vector<String> addresses_ipv4;
      std::vector<String> addresses_ipv6;

      while (endpoint_iterator != tcp::resolver::iterator())   
      {
         tcp::endpoint endpoint = *endpoint_iterator;
         boost::asio::ip::address adr = endpoint.address();

         std::string result = adr.to_string(errorCode);

         if (errorCode)
         {
            String sMessage;
            sMessage.Format(_T("DNS query failure. Treating as temporary failure. Conversion of DNS record to string failed. Domain: %s, Error code: %d, Message: %s"), sDomain.c_str(), errorCode.value(), String(errorCode.message()).c_str());
            LOG_TCPIP(sMessage);
            return false;
         }

         if (adr.is_v4())
            addresses_ipv4.push_back(result);
         if (adr.is_v6())
            addresses_ipv6.push_back(result);

         endpoint_iterator++;
      }

      // if IPv6 is preferred first return IPv6 DNS Lookup(s)
      if (TCPServer::HasIPV6() && Configuration::Instance()->GetIPv6Preferred())
      {
         for (String address : addresses_ipv6)
            saFoundNames.push_back(address);
         for (String address : addresses_ipv4)
            saFoundNames.push_back(address);
      }
      else
      {
         for (String address : addresses_ipv4)
            saFoundNames.push_back(address);
         for (String address : addresses_ipv6)
            saFoundNames.push_back(address);
      }

      return true;
   }

   bool 
   DNSResolver::GetTXTRecords(const String &sDomain, std::vector<String> &foundResult)
   {
      return Resolve_(sDomain, foundResult, DNS_TYPE_TEXT, 0);
   }

   bool
   DNSResolver::GetEmailServers(const String &sDomainName, std::vector<HostNameAndIpAddress> &saFoundNames )
   {
      String message = Formatter::Format("DNS MX lookup: {0}", sDomainName);
      LOG_TCPIP(message);

      std::vector<String> vecFoundMXRecords;

      if (!Resolve_(sDomainName, vecFoundMXRecords, DNS_TYPE_MX, 0))
      {
         String logMessage;
            logMessage.Format(_T("Failed to resolve email servers (MX lookup). Domain name: %s."), sDomainName.c_str());
         LOG_DEBUG(logMessage);

         return false;
      }

      if (vecFoundMXRecords.empty())
      {
         /* RFC 2821:
            If no MX records are found, but an A RR is found, the A RR is treated as
            if it was associated with an implicit MX RR, with a preference of 0,
            pointing to that host.  If one or more MX RRs are found for a given
            name, SMTP systems MUST NOT utilize any A RRs associated with that
            name unless they are located using the MX RRs;
            (implemented here)
         */

         std::vector<String> a_records;
         if (!GetARecords(sDomainName, a_records))
         {
            String logMessage;
               logMessage.Format(_T("Failed to resolve email servers (A lookup). Domain name: %s."), sDomainName.c_str());
            LOG_DEBUG(logMessage);

            return false;
         }
		 
		 for (String record : a_records)         
		 {
		 	HostNameAndIpAddress hostAndAddress;
            hostAndAddress.SetHostName(sDomainName);
            hostAndAddress.SetIpAddress(record);

            saFoundNames.push_back(hostAndAddress);
         }
      }
      else
      {
         // We've been able to find host names in the MX records. We should
         // now translate them to IP addresses. Some host names may result
         // in several IP addreses.
         auto iterDomain = vecFoundMXRecords.begin();

         bool dnsSuccess = false;
         for(String domain : vecFoundMXRecords)
         {
            // Resolve to domain name to IP address and put it in the list.
            int iCountBefore = saFoundNames.size();

            std::vector<String> a_records;
            if (!GetARecords(domain, a_records))
               continue;

            dnsSuccess = true;
            
            if (saFoundNames.size() == iCountBefore)
            {
               // No mx records were found for this host name. Check if the host
               // name is actually an IP address? It shouldn't be but....

               if (StringParser::IsValidIPAddress(domain))
               {
                  // Okay, this is an invalid MX record. The MX record should always contain 
                  // a host name but in this case it appears an IP address. We'll be kind to
                  // the domain owner and still deliver the email to him.
                  a_records.push_back(domain);
               }
            }

            for (String record : a_records)
            {
               HostNameAndIpAddress hostAndAddress;
               hostAndAddress.SetHostName(domain);
               hostAndAddress.SetIpAddress(record);

               saFoundNames.push_back(hostAndAddress);
            }
         }

         if (!dnsSuccess)
         {
            // All dns queries failed.
            String logMessage;
               logMessage.Format(_T("Failed to resolve email servers (A lookup). Domain name: %s."), sDomainName.c_str());
            LOG_DEBUG(logMessage);

            return false;
         }

      }

      String sLogMsg;
      sLogMsg.Format(_T("DNS - MX Result: %d IP addresses were found."), saFoundNames.size());

      LOG_TCPIP(sLogMsg);

            
      // Remove duplicate names.
      auto iter = saFoundNames.begin();
      std::set<String> duplicateCheck;

      while (iter != saFoundNames.end())
      {
         String name = (*iter).GetIpAddress();
         if (duplicateCheck.find(name) != duplicateCheck.end())
         {
            // We found a duplicate. Remove it.
            iter = saFoundNames.erase(iter);
         }
         else
         {
            // This is not a duplicate. Move to next.
            iter++;

            duplicateCheck.insert(name);
         }
      }

      return true;
   }

   bool 
   DNSResolver::GetMXRecords(const String &sDomain, std::vector<String> &vecFoundNames)
   {
      return Resolve_(sDomain, vecFoundNames, DNS_TYPE_MX, 0);
   }

   bool 
   DNSResolver::GetPTRRecords(const String &sIP, std::vector<String> &vecFoundNames)
   {
      IPAddress address;
      if (!address.TryParse(AnsiString(sIP), true))
         return false;

      if (address.GetType() == IPAddress::IPV4)
      {
         std::vector<String> vecItems = StringParser::SplitString(sIP, ".");
         reverse(vecItems.begin(), vecItems.end());
         String result = StringParser::JoinVector(vecItems, ".");
         return Resolve_(result + ".in-addr.arpa", vecFoundNames, DNS_TYPE_PTR, 0);
      }
      else
      {
         AnsiString long_ipv6 = address.ToLongString();
         long_ipv6.MakeReverse();
         long_ipv6.Remove(':');

         for (int i = long_ipv6.GetLength() - 1; i > 0; i--)
         {
            long_ipv6.insert(i, 1, '.');
         }

         return Resolve_(long_ipv6 + ".ip6.arpa", vecFoundNames, DNS_TYPE_PTR, 0);
      }


   }

}

