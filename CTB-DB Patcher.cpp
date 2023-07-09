#include <iostream>
#include <string>
#include <vector>
#include <shobjidl_core.h>
#include "sqlite3.h"
#include "json.hpp"

using json = nlohmann::json;

class Database {
public:
	Database(const std::string& databaseFile) : db(nullptr) {
		int result = sqlite3_open(databaseFile.c_str(), &db);
		if (result != SQLITE_OK) {
			std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			db = nullptr;
		}
	}

	~Database() {
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
	}

	bool isOpen() const {
		return db != nullptr;
	}

	bool executeQuery(const std::string& query) const {
		char* errMsg = nullptr;
		int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
		if (result != SQLITE_OK) {
			std::cerr << "Error executing query: " << errMsg << std::endl;
			sqlite3_free(errMsg);
			return false;
		}
		return true;
	}

	bool executeSelectQuery(const std::string& query, std::vector<std::vector<std::string>>& results) const {
		char* errMsg = nullptr;
		results.clear();
		int result = sqlite3_exec(db, query.c_str(), selectCallback, &results, &errMsg);
		if (result != SQLITE_OK) {
			std::cerr << "Error executing select query: " << errMsg << std::endl;
			sqlite3_free(errMsg);
			return false;
		}
		return true;
	}

private:
	sqlite3* db;

	static int selectCallback(void* data, int argc, char** argv, char** azColName) {
		auto results = static_cast<std::vector<std::vector<std::string>>*>(data);
		std::vector<std::string> row;
		for (int i = 0; i < argc; i++) {
			row.push_back(argv[i] ? argv[i] : "NULL");
		}
		results->push_back(row);
		return 0;
	}
};

class Application {
public:
	void run() {
		std::string databasePath = "C:\\ProgramData\\Cold Turkey\\data-app.db";

		// Initialize the COM library
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr)) {
			std::cerr << "Error initializing COM library: " << hr << std::endl;
			return;
		}

		if (fileExists(databasePath)) {
			processDatabase(databasePath);
		}
		else {
			std::string selectedFile = openFileDialog();
			if (selectedFile.empty()) {
				std::cerr << "No database file selected." << std::endl;
				return;
			}

			processDatabase(selectedFile);
		}
	}

private:
	bool fileExists(const std::string& filePath) {
		std::wstring wFilePath(filePath.begin(), filePath.end());
		DWORD fileAttributes = GetFileAttributesW(wFilePath.c_str());
		return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	}

	void processDatabase(const std::string& databaseFile) {
		Database database(databaseFile);
		if (!database.isOpen()) {
			std::cerr << "Failed to open the database." << std::endl;
			return;
		}

		std::string selectQuery = "SELECT value FROM settings WHERE key = 'settings';";
		std::vector<std::vector<std::string>> results;
		if (database.executeSelectQuery(selectQuery, results)) {
			if (results.size() == 1 && results[0].size() == 1) {
				std::string jsonValue = results[0][0];

				try {
					json jsonData = json::parse(jsonValue);

					// Modify the "proStatus" value here
					jsonData["additional"]["proStatus"] = "pro";

					std::string updatedJson = jsonData.dump();

					std::string updateQuery = "UPDATE settings SET value = '" + updatedJson + "' WHERE key = 'settings';";
					if (database.executeQuery(updateQuery)) {
						showSuccess("Database patched successfully.");
					}
					else {
						std::cerr << "Failed to execute the update query." << std::endl;
					}
				}
				catch (const std::exception& e) {
					std::cerr << "Error parsing JSON data: " << e.what() << std::endl;
				}
			}
			else {
				std::cerr << "Invalid data retrieved from the settings table." << std::endl;
			}
		}
		else {
			std::cerr << "Failed to execute the select query." << std::endl;
		}
	}

	std::string openFileDialog() {
		std::string filePath;

		IFileOpenDialog* pFileOpen;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen)))) {
			DWORD dwOptions;
			if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
				pFileOpen->SetOptions(dwOptions | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

				pFileOpen->SetTitle(L"Select Database File");

				if (SUCCEEDED(pFileOpen->Show(nullptr))) {
					IShellItem* pItem;
					if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
						PWSTR pszFilePath;
						if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
							int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, nullptr, 0, nullptr, nullptr);
							if (bufferSize > 0) {
								std::vector<char> buffer(bufferSize);
								WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, buffer.data(), bufferSize, nullptr, nullptr);
								filePath = buffer.data();
							}
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
			}
			pFileOpen->Release();
		}

		return filePath;
	}

	void showSuccess(const std::string& errorMessage) const {
		MessageBoxA(nullptr, errorMessage.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
		exit(1);
	}
};

int main() {
	Application app;
	app.run();

	return 0;
}