/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Josekutty Kuriakose
 */

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <system_error>

#include <boost/program_options.hpp>

#include <ralf/Package.h>
#include <ralf/VerificationBundle.h>

using namespace LIBRALF_NS;
namespace po = boost::program_options;

std::string mapMetadataType(PackageType type);
std::string printPermissions(const Permissions &permissions);

void printAudioInfo(const AudioInfo &audioInfo);
void printDialInfo(const DialInfo &dialInfo);
void printInputHandlingInfo(const InputHandlingInfo &inputInfo);
void printVendorConfigKeys(const PackageMetaData &metadata);
void printSigningCertificates(const Package &package);
void printApplicationInfo(const ApplicationInfo &appInfo);

const std::string SEPARATOR_LINE(60, '*');

bool loadCertificateFromFile(const std::string &certPath, VerificationBundle &bundle)
{
	bool status = false;

	std::cout << "Loading certificate from: " << certPath << std::endl;
	auto certPem = Certificate::loadFromFile(certPath, Certificate::EncodingFormat::PEM);
	if (certPem)
	{
		std::cout << "Loaded certificate in PEM format from: " << certPath << std::endl;
		bundle.addCertificate(certPem.value());
		status = true;
	}
	else
	{
		auto certDer = Certificate::loadFromFile(certPath, Certificate::EncodingFormat::DER);
		if (certDer)
		{
			std::cout << "Loaded certificate in DER format from: " << certPath << std::endl;
			bundle.addCertificate(certDer.value());
			status = true;
		}
		else
		{
			std::cerr << "Failed to load certificate: " << certPath
					  << " (PEM error: " << certPem.error().what()
					  << ", DER error: " << certDer.error().what() << ")" << std::endl;
		}
	}
	return status;
}
bool loadCertificatesFromDirectory(const std::string &dirPath, VerificationBundle &bundle)
{
	// Check if the path exists
	if (!std::filesystem::exists(dirPath))
	{
		std::cerr << "Error: Certificate directory does not exist: " << dirPath << std::endl;
		return false;
	}

	// Check if the path is actually a directory
	if (!std::filesystem::is_directory(dirPath))
	{
		std::cerr << "Error: Certificate path is not a directory: " << dirPath << std::endl;
		return false;
	}

	std::error_code ec;
	std::filesystem::directory_iterator dirIter(dirPath, ec);
	if (ec)
	{
		std::cerr << "Error accessing directory: " << ec.message() << std::endl;
		return false;
	}

	// Set of valid certificate file extensions
	const std::set<std::string> validExtensions = {".pem", ".crt", ".cer", ".cert", ".der"};

	for (const auto &entry : dirIter)
	{
		// Skip symbolic links with a warning
		if (entry.is_symlink())
		{
			std::cerr << "Warning: Skipping symbolic link: " << entry.path().string() << std::endl;
			continue;
		}

		if (entry.is_regular_file())
		{
			const std::string certPath = entry.path().string();
			std::string extension = entry.path().extension().string();
			
			// Convert extension to lowercase for case-insensitive comparison
			std::transform(extension.begin(), extension.end(), extension.begin(),
						   [](unsigned char c) { return std::tolower(c); });

			// Skip files that don't have a valid certificate extension
			if (validExtensions.find(extension) == validExtensions.end())
			{
				continue;
			}

			if (!loadCertificateFromFile(certPath, bundle))
			{
				std::cerr << "Failed to load certificate from file, ignoring: " << certPath << std::endl;
				continue;
			}
		}
	}
	return bundle.certificates().size() > 0;
}
bool loadCertificates(const std::string &certPath, const std::string &certDir, VerificationBundle &verificationBundle)
{
	if (!certPath.empty())
	{
		if (!loadCertificateFromFile(certPath, verificationBundle))
		{
			std::cerr << "Failed to load certificate from file: " << certPath << std::endl;
			return false;
		}
	}
	else if (!certDir.empty())
	{
		if (!loadCertificatesFromDirectory(certDir, verificationBundle))
		{
			std::cerr << "Failed to load certificates from directory: " << certDir << std::endl;
			return false;
		}
	}
	else
	{
		std::cerr << "No certificate provided for verification." << std::endl;
		return false;
	}
	return true;
}
void printMetadataInformation(const PackageMetaData &metadata)
{

	std::cout << "Metadata information:\n"
			  << SEPARATOR_LINE << std::endl;
	std::cout << "ID " << metadata.id() << std::endl;
	std::cout << "Version " << metadata.version().toString() << std::endl;
	std::cout << "Package Type " << mapMetadataType(metadata.type()) << std::endl;
	std::cout << "Mime Type " << metadata.mimeType() << std::endl;
	std::cout << "Title " << metadata.title().value_or("N/A") << std::endl;
	std::cout << "Entry Point Path " << metadata.entryPointPath().string() << std::endl;
	std::cout << "Entry Args ";
	for (const auto &arg : metadata.entryArgs())
	{
		std::cout << arg << " ";
	}
	std::cout << std::endl;
	std::cout << SEPARATOR_LINE << std::endl;
}

void printMetadataDependenciesInfo(const std::map<std::string, VersionConstraint> &dependencies)
{
	std::cout << " Dependencies: " << std::endl;
	if (dependencies.empty())
	{
		std::cout << "  None" << std::endl;
		return;
	}
	for (auto &dep : dependencies)
	{
		std::cout << "ID: " << dep.first << ", Version Constraint: " << dep.second.toString() << std::endl;
	}
}

void printApplicationIconDetails(const std::list<Icon> &icons)
{
	std::cout << "Icons: " << std::endl;
	if (icons.empty())
	{
		std::cout << "None" << std::endl;
		return;
	}
	for (const auto &icon : icons)
	{
		std::cout << "Path: " << icon.path.string() << ", MimeType: " << icon.mimeType << ", Purpose: " << icon.purpose << ", Sizes: ";
		for (const auto &size : icon.sizes)
		{
			std::cout << size.first << "x" << size.second << " ";
		}
		std::cout << std::endl;
	}
}

int main(int argc, char *argv[])
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "Produce help message")
		("verify,v", "Explicitly enable package signature verification. Requires certificates via --cert or --cert-dir (verification fails if used without either option). Automatically enabled if --cert or --cert-dir is provided.")
		("cert,c", po::value<std::string>(), "Path to the certificate file (enables verification)")
		("cert-dir,d", po::value<std::string>(), "Path to directory containing certificates (enables verification)")
		("package_file,p", po::value<std::string>(), "Path to the package file")
		("mount-package,m", po::value<std::string>(), "Mount the package if it is mountable");

	po::positional_options_description p;
	p.add("package_file", 1);

	po::variables_map vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);
	}
	catch (const po::error &ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 0;
	}

	if (!vm.count("package_file"))
	{
		std::cerr << "Package file is required." << std::endl;
		std::cerr << desc << std::endl;
		return 1;
	}

	std::string packageFile = vm["package_file"].as<std::string>();
	std::string certPath = vm.count("cert") ? vm["cert"].as<std::string>() : "";
	std::string certDir = vm.count("cert-dir") ? vm["cert-dir"].as<std::string>() : "";
	std::string mountPath = vm.count("mount-package") ? vm["mount-package"].as<std::string>() : "";

	// Enable verification if --verify is specified OR if --cert or --cert-dir is provided
	bool verify = vm.count("verify") || !certPath.empty() || !certDir.empty();

	if (!certPath.empty() && !certDir.empty())
	{
		std::cerr << "Error: Cannot specify both --cert and --cert-dir. Use only one." << std::endl;
		return 1;
	}

	// Create a VerificationBundle if verification is enabled
	VerificationBundle verificationBundle;
	if (verify)
	{
		if (!loadCertificates(certPath, certDir, verificationBundle))
		{
			std::cerr << "Failed to load certificates for verification." << std::endl;
			return 1;
		}
	}

	auto package = verify
					   ? Package::open(packageFile, verificationBundle, Package::OpenFlags::None)
					   : Package::openWithoutVerification(packageFile);

	if (!package)
	{
		std::cerr << "Failed to open package: " << package.error().what() << std::endl;
		return 1;
	}

	std::cout << "Successfully opened package: " << packageFile << std::endl;
	std::cout << "Package information:\n"
			  << SEPARATOR_LINE << std::endl;
	std::cout << "Package format: " << package->format() << std::endl;
	std::cout << "Package ID: " << package->id() << std::endl;
	std::cout << "Package version: " << package->version().toString() << std::endl;
	std::cout << " Is Mountable ? " << (package->isMountable() ? "Yes" : "No") << std::endl;

	if (package->isMountable() && !mountPath.empty())
	{
		auto mountResult = package->mount(mountPath);
		if (!mountResult)
		{
			std::cerr << "Failed to mount package at " << mountPath << ": " << mountResult.error().what() << std::endl;
			return 1;
		}

		std::cout << "Package mounted successfully at: " << mountPath << std::endl;
		std::cout << "DM device name: " << mountResult->volumeName() << std::endl;
		std::cout << "DM device uuid: " << mountResult->volumeUuid() << std::endl;
		std::cout << "Will be unmounted automatically when the program exits." << std::endl;
	}
	else if (!package->isMountable() && !mountPath.empty())
	{
		std::cerr << "Warning: mount path '" << mountPath
				  << "' was provided, but the package is not mountable. "
				  << "The package will not be mounted." << std::endl;
	}

	printSigningCertificates(package.value());

	auto metadata = package->metaData();

	if (!metadata)
	{
		std::cout << "Failed to get metadata: " << metadata.error().what() << std::endl;
		return 1;
	}
	printMetadataInformation(metadata.value());
	printMetadataDependenciesInfo(metadata->dependencies());

	printApplicationIconDetails(metadata->icons());

	auto appInfo = metadata->applicationInfo();
	if (appInfo)
	{
		std::cout << " Application Info Present: Yes" << std::endl;
		printApplicationInfo(appInfo.value());
	}
	else
	{
		std::cout << " No Application Info present." << std::endl;
	}

	auto serviceInfo = metadata->serviceInfo();
	std::cout << " Service Info Present: " << (serviceInfo ? "Yes" : "No") << std::endl;

	auto runtimeInfo = metadata->runtimeInfo();
	std::cout << " Runtime Info: Present " << (runtimeInfo ? "Yes" : "No") << std::endl;

	printVendorConfigKeys(metadata.value());

	std::cout << "Auxiliary metadata keys:" << std::endl;
	auto auxKeysResult = package->auxMetaDataKeys();
	if (auxKeysResult)
	{
		auto keys = auxKeysResult.value();
		for (const auto &key : keys)
			std::cout << "  " << key << std::endl;
	}
	else
	{
		std::cout << "  Failed to get aux keys: " << auxKeysResult.error().what() << std::endl;
	}
	std::cout << SEPARATOR_LINE << std::endl;
	return 0;
}
void printApplicationInfo(const ApplicationInfo &app)
{
	std::cout << " Application Info: " << std::endl;

	std::cout << "  Runtime Type: " << app.runtimeType() << std::endl;
	auto permissions = app.permissions();
	std::cout << "  Permissions: " << printPermissions(permissions) << std::endl;
	auto storageQuota = app.storageQuota();
	std::cout << "  Storage Quota: " << storageQuota.value_or(0L) << " bytes" << std::endl;
	auto memoryQuota = app.memoryQuota();
	std::cout << "  Memory Quota: " << memoryQuota.value_or(0L) << " bytes" << std::endl;
	auto gpuMemoryQuota = app.gpuMemoryQuota();
	std::cout << "  GPU Memory Quota: " << gpuMemoryQuota.value_or(0L) << " bytes" << std::endl;

	auto inputInfoResult = app.inputHandlingInfo();
	if (inputInfoResult)
	{
		auto inputInfo = inputInfoResult.value();
		printInputHandlingInfo(inputInfo);
	}

	printAudioInfo(app.audioInfo());

	auto dialInfoResult = app.dialInfo();
	if (dialInfoResult)
		printDialInfo(dialInfoResult.value());
}
void printSigningCertificates(const Package &package)
{
	auto certInfo = package.signingCertificates();
	if (certInfo)
	{
		auto certs = certInfo.value();
		for (const auto &cert : certs)
		{
			std::cout << "Certificate Subject: " << cert.subject() << std::endl;
			std::cout << "Certificate Issuer: " << cert.issuer() << std::endl;
			std::cout << "Certificate Is Valid: " << cert.isValid() << std::endl;
		}
	}
	else
	{
		std::cout << "Failed to get signing certificates: " << certInfo.error().what() << std::endl;
	}
}
std::string mapMetadataType(PackageType type)
{
	switch (type)
	{
	case PackageType::Unknown:
		return "Unknown";
	case PackageType::Application:
		return "Application";
	case PackageType::Service:
		return "Service";
	case PackageType::Runtime:
		return "Runtime";
	default:
		return "Invalid";
	}
}

std::string printPermissions(const Permissions &permissions)
{
	std::set<std::string> perms = permissions.all();
	if (perms.empty())
		return "None";

	std::string result;
	for (const auto &perm : perms)
	{
		if (!result.empty())
			result += ", ";
		result += perm;
	}
	return result;
}
void printInputHandlingInfo(const InputHandlingInfo &inputInfo)
{
	std::cout << "  Input Handling Info: " << std::endl;
	std::cout << "   Intercepted Keys: ";
	for (const auto &key : inputInfo.interceptedKeys)
	{
		std::cout << key << " ";
	}
	std::cout << std::endl;
	std::cout << "   Captured Keys: ";
	for (const auto &key : inputInfo.capturedKeys)
	{
		std::cout << key << " ";
	}
	std::cout << std::endl;
	std::cout << "   Monitored Keys: ";
	for (const auto &key : inputInfo.monitoredKeys)
	{
		std::cout << key << " ";
	}
	std::cout << std::endl;
}
void printAudioInfo(const AudioInfo &audioInfo)
{
	std::cout << "  Audio Info: " << std::endl;
	std::cout << "   Sound Mode: " << audioInfo.soundMode.value_or("N/A") << std::endl;
	std::cout << "   Sound Scene: " << audioInfo.soundScene.value_or("N/A") << std::endl;
	std::cout << "   Loudness Adjustment: " << (audioInfo.loudnessAdjustment ? std::to_string(audioInfo.loudnessAdjustment.value()) : "N/A") << std::endl;
}
void printDialInfo(const DialInfo &dialInfo)
{

	std::cout << "  DIAL Info: " << std::endl;
	auto dialIds = dialInfo.dialIds;
	std::cout << "   DIAL IDs: ";
	for (const auto &id : dialIds)
	{
		std::cout << id << " ";
	}
	std::cout << std::endl;
	auto corsOriginDomains = dialInfo.corsDomains;
	std::cout << "   CORS Domains: ";
	for (const auto &domain : corsOriginDomains)
	{
		std::cout << domain << " ";
	}
	std::cout << std::endl;
}
void printVendorConfigKeys(const PackageMetaData &metadata)
{
	std::cout << "  Vendor Config Keys: " << std::endl;
	auto keys = metadata.vendorConfigKeys();
	for (const auto &key : keys)
	{
		auto value = metadata.vendorConfig(key); // just to test retrieval
		std::cout << "    " << key << " <<" << value << ">>" << std::endl;
	}
}
