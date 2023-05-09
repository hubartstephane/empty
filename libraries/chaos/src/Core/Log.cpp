#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"


namespace chaos
{
	static std::vector<std::pair<LogType, char const*>> const log_type_encoding =
	{
		{ LogType::Message, "MESSAGE" },
		{ LogType::Warning, "WARNING" },
		{ LogType::Error,   "ERROR" }
	};

	CHAOS_IMPLEMENT_ENUM_METHOD(LogType, log_type_encoding);

	std::string LogLine::ToString() const
	{
		//return std::format("{0} {1} {2}", EnumToString(type), 9, 3);

		return {};
	}

	bool LogLine::IsComparable(LogLine const& src) const
	{
		// compare members to members, by comparaison speed (ignore time)
		return
			(type == src.type) &&
			(domain == src.domain) && // raw pointer comparaison
			(content == src.content);
	}

	Log* Log::GetInstance()
	{
		// XXX : use a share pointer so that we are sure it is being destroyed at the end of the application (and so the output_file is being flushed)
		static shared_ptr<Log> result; 
		if (result == nullptr)
			result = new Log();
		return result.get();
	}

	void Log::DoFormatAndOuput(LogType type, bool add_line_jump, char const* format, ...)
	{
		va_list va;
		va_start(va, format);

		// format the message
		char buffer[4096];
		vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, va); // doesn't count for the zero  
		// output the message
		DoOutput(type, add_line_jump, buffer);

		va_end(va);
	}

	boost::filesystem::path Log::GetOutputPath()
	{
		if (Application* application = Application::GetInstance())
		{
			return application->GetUserLocalTempPath() / "logs.txt";
		}
		return {};
	}

	void Log::DoOutput(LogType type, bool add_line_jump, std::string_view buffer)
	{
		// register the new line
		LogLine new_line;
		new_line.type = type;
		new_line.content = buffer;
		new_line.time = std::chrono::system_clock::now();
		lines.push_back(new_line);

		// output in standard output
		std::ostream& output = (type == LogType::Error) ? std::cerr : std::cout;
		output << buffer;
		if (add_line_jump)
			output << "\n";
		// generate output in file
		if (open_output_file && !output_file.is_open())
		{
			// even in case of failure do not ever try to open the file
			open_output_file = false; 
			// open the file
			boost::filesystem::path log_path = GetOutputPath();
			if (!log_path.empty())
			{
				output_file.open(log_path.c_str(), std::ofstream::binary | std::ofstream::trunc);
			}
		}
		// output in file
		if (output_file.is_open())
		{
			output_file << buffer;
			if (add_line_jump)
				output_file << "\n";
		}
	}

	void Log::Title(char const* title)
	{
		assert(title != nullptr);

		// get the logger
		Log* log = GetInstance();
		if (log == nullptr)
			return;

		// fill separator buffer
		char line[512] = { 0 };

		size_t l = 12 + strlen(title);
		if (l < sizeof(line) - 1)
			for (size_t i = 0; i < l; ++i)
				line[i] = '=';

		// output
		log->DoOutput(LogType::Message, true, "");
		if (l < sizeof(line) - 1)
			log->DoOutput(LogType::Message, true, line);

		log->DoOutput(LogType::Message, false, "===   ");
		log->DoOutput(LogType::Message, false, title);
		log->DoOutput(LogType::Message, true, "   ===");

		if (l < sizeof(line) - 1)
			log->DoOutput(LogType::Message, true, line);
		log->DoOutput(LogType::Message, true, "");
	}
}; // namespace chaos
