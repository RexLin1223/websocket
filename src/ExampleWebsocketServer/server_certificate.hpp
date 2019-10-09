//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

/*  Load a signed certificate into the ssl context, and configure
	the context for use with a server.

	For this to work with the browser or operating system, it is
	necessary to import the "Beast Test CA" certificate into
	the local certificate store, browser, or operating system
	depending on your environment Please see the documentation
	accompanying the Beast certificate for more details.
*/
inline
void
load_server_certificate(boost::asio::ssl::context& ctx)
{
	/*
		The certificate was generated from CMD.EXE on Windows 10 using:

		winpty openssl dhparam -out dh.pem 2048
		winpty openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "//C=US\ST=CA\L=Los Angeles\O=Beast\CN=www.example.com"
	*/

	std::string const cert =
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
		"-----END CERTIFICATE-----\n";*/
		"-----BEGIN CERTIFICATE-----\n"
		"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\n"
		"mxUpZ/YetsaBvy8T8dZ3ds962iSD/zFHjw==\n"
		"-----END CERTIFICATE-----\n";

	std::string const key =
		/*"-----BEGIN PRIVATE KEY-----\n"
		"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
		"Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
		"GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
		"fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
		"KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
		"sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
		"Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
		"oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
		"1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
		"OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
		"VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
		"bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
		"PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
		"VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
		"CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
		"Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
		"CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
		"VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
		"GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
		"zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
		"/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
		"hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
		"23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
		"1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
		"k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
		"pVOUFq5mW8p0zbtfHbjkgxyF\n"
		"-----END PRIVATE KEY-----\n";
		*/

		"-----BEGIN RSA PRIVATE KEY-----\n"
		"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\n"
		"-----END RSA PRIVATE KEY-----\n";

	std::string const dh =
		"-----BEGIN DH PARAMETERS-----\n"
		"MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
		"/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
		"4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
		"tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
		"oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
		"QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
		"-----END DH PARAMETERS-----\n";

	ctx.set_password_callback(
		[](std::size_t,
			boost::asio::ssl::context_base::password_purpose)
	{
		return "123456";
	});

	ctx.set_options(
		boost::asio::ssl::context::default_workarounds |
		boost::asio::ssl::context::no_sslv2 |
		boost::asio::ssl::context::no_sslv3);

	try {
		ctx.use_certificate_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependencies\host.crt)", boost::asio::ssl::context::file_format::pem);
		ctx.use_rsa_private_key_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependencies\host.key)", boost::asio::ssl::context::file_format::pem);
		//ctx.use_certificate_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependency\cert.pem)", boost::asio::ssl::context::file_format::pem);
		//ctx.use_rsa_private_key_file(R"(C:\Users\Rex\Documents\Visual Studio 2017\Projects\web_socket_server\dependency\key.pem)", boost::asio::ssl::context::file_format::pem);
	}
	catch (std::exception& ec) {
		printf(ec.what());
	}

	/*ctx.use_certificate_chain(
		boost::asio::buffer(cert.data(), cert.size()));

	ctx.use_private_key(
		boost::asio::buffer(key.data(), key.size()),
		boost::asio::ssl::context::file_format::pem);*/

	/*ctx.use_tmp_dh(
		boost::asio::buffer(dh.data(), dh.size()));*/
}

#endif