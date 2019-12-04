//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_ROOT_CERTIFICATES_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_ROOT_CERTIFICATES_HPP

#include <boost/asio/ssl.hpp>
#include <string>

/*
	PLEASE READ

	These root certificates here are included just to make the
	SSL client examples work. They are NOT intended to be
	illustrative of best-practices for performing TLS certificate
	verification.

	A REAL program which needs to verify the authenticity of a
	server IP address resolved from a given DNS name needs to
	consult the operating system specific certificate store
	to validate the chain of signatures, compare the domain name
	properly against the domain name in the certificate, check
	the certificate revocation list, and probably do some other
	things.

	ALL of these operations are entirely outside the scope of
	both Boost.Beast and Boost.Asio.

	See (work in progress):
		https://github.com/djarek/certify

	tl;dr: root_certificates.hpp should not be used in production code
*/

namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

namespace detail {

	inline
		void
		load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
	{
		std::string const caCert =
			/*  This is the GeoTrust root certificate.

				CN = GeoTrust Global CA
				O = GeoTrust Inc.
				C = US
				Valid to: Friday, ‎May ‎20, ‎2022 9:00:00 PM

				Thumbprint(sha1):
				‎de 28 f4 a4 ff e5 b9 2f a3 c5 03 d1 a3 49 a7 f9 96 2a 82 12
			*/			
			/*"-----BEGIN CERTIFICATE-----\n"
			"MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
			"BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
			"Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
			"MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
			"ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
			"A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
			"xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
			"Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
			"9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
			"yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
			"enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
			"BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
			"4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
			"LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
			"gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
			"V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
			"fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
			"9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
			"UEVbkhd5qstF6qWK\n"
			"-----END CERTIFICATE-----\n";	
			*/
			"-----BEGIN CERTIFICATE-----\n"
			"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\n"
			"+jRjKCKePaRWKDBcNWoWzYE=\n"
			"-----END CERTIFICATE-----\n";

		std::string const cert =
			"-----BEGIN CERTIFICATE-----\n"
			"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\n"
			"mxUpZ/YetsaBvy8T8dZ3ds962iSD/zFHjw==\n"
			"-----END CERTIFICATE-----\n";

		/*ctx.add_certificate_authority(
			boost::asio::buffer(caCert.data(), caCert.size()), ec);
		if (ec)
			return;*/
		//ctx.load_verify_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependency\ca.crt)");
		ctx.load_verify_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependency\host.crt)");
		//ctx.use_certificate_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependency\host.crt)", boost::asio::ssl::context::file_format::pem);

		if (ec)
			return;
	}

} // detail

// Load the root certificates into an ssl::context

inline
void
load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
{
	detail::load_root_certificates(ctx, ec);
}

inline
void
load_root_certificates(ssl::context& ctx)
{
	boost::system::error_code ec;
	detail::load_root_certificates(ctx, ec);
	if (ec)
		throw boost::system::system_error{ ec };
}

#endif