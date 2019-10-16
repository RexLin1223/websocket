#pragma once
#include <iostream>
#include <string>

#include <mutex>
#include <shared_mutex>

#include <chrono>
#include <ctime>
#include <locale>

#include <list>
#include <vector>
#include <functional>
#include <sstream>
#include <stdarg.h>
#include <thread>

#ifndef _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#endif

namespace websocket {
	// Time 
	static std::string GetNow() {

		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string s(24, '\0');
		std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

		return s;
	}

	static uint64_t GetNowEpoch() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	// Logger
	template<typename T>
	void exception_log(char const* category, T ec)
	{
		std::cerr << GetNow() << category << ": " << ec.message() << "\n";
	}

	static void log(const std::string& content) {
		std::cout << GetNow() << content << std::endl;
	}

	/*
	// Variadic functions
	void log(const char* fmt, ...) {
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, 1024, fmt, args);
		va_end(args);

		log(std::string(buf, 1024));
	}
	*/
	static void templateLogImpl(std::stringstream& os, const char* format)
	{
		os << format;
	}

	template<typename T, typename... Targs>
	static void templateLogImpl(std::stringstream& os, const char* fmt, T value, Targs... Fargs) {
		for (; *fmt != '\0'; fmt++) {
			if (*fmt == '%') {
				os << value;
				templateLogImpl(os, fmt + 2, Fargs...);
				return;
			}
			os << *fmt;
		}
	}
	// Wrapper of Parameter pack template
	// Parameter pack template
	template<typename T, typename... Targs>
	void log(const char* fmt, T value, Targs... Fargs)
	{
		std::stringstream ss;
		templateLogImpl(ss, fmt, value, Fargs...);

		log(ss.str());
	}

	template<typename T>
	class LockQueue {
		std::mutex mutex_;
		std::list<T> queue_;
	public:
		LockQueue() {}
		virtual ~LockQueue() {}

		std::list<T>& get() {
			return this->queue_;
		}

		void push(const T& obj) {
			std::lock_guard<std::mutex> guard(mutex_);
			queue_.emplace_back(obj);
		}

		void push(T&& obj) {
			std::lock_guard<std::mutex> guard(mutex_);
			queue_.emplace_back(std::move(obj));
		}

		void consume_one() {
			std::lock_guard<std::mutex> guard(mutex_);
			queue_.pop_front();
		}
		
		void consume_all() {
			std::lock_guard<std::mutex> guard(mutex_);
			queue_.clear();
		}
		/*
		T fetch() {
			std::lock_guard<std::mutex> guard(mutex_);
			T obj = queue_.front();
			queue_.pop_front();
			return obj;
		}
		*/

		T&& fetch() {
			std::lock_guard<std::mutex> guard(mutex_);
			T obj = std::move(queue_.front());
			queue_.pop_front();
			return std::move(obj);
		}

		T& peek() {
			std::lock_guard<std::mutex> guard(mutex_);
			return queue_.front();
		}

		size_t size() {
			std::lock_guard<std::mutex> guard(mutex_);
			return queue_.size();
		}

		bool empty() {
			std::lock_guard<std::mutex> guard(mutex_);
			return 0 == queue_.size();
		}
	};

	template<typename T> using CallbackHandler = std::function<void(const T&)>;
	template<typename T>
	class MessageChannel {
		std::shared_mutex  mutex_;
		std::vector<T> subscribers_;
	public:
		MessageChannel() {}
		virtual ~MessageChannel() {
			std::lock_guard<std::shared_mutex> guard(mutex_);
			subscribers_.clear();
		}

		void subscribe(const T& callback) {
			std::lock_guard<std::shared_mutex> guard(mutex_);
			subscribers_.emplace_back(callback);
		}

		void subscribe(T&& callback) {
			std::lock_guard<std::shared_mutex> guard(mutex_);
			subscribers_.emplace_back(std::move(callback));
		}

		template<typename T2>
		void broadcast(const T2& data) {
			std::shared_lock<std::shared_mutex> guard(mutex_);
			for (auto cb : subscribers_) {
				if (cb) {
					cb(data);
				}
			}
		}

		template<typename T2>
		void broadcast(T2&& data) {
			std::shared_lock<std::shared_mutex> guard(mutex_);
			for (auto cb : subscribers_) {
				if (cb) {
					cb(data);
				}
			}
		}
	};
	using Channel = MessageChannel<CallbackHandler<std::string>>;

	class ThreadGroup {
		std::list<std::thread> threads_;
		mutable std::mutex  mutex_;
	private:
		ThreadGroup(ThreadGroup const&);
		ThreadGroup& operator=(ThreadGroup const&);
	public:
		ThreadGroup() {}
		~ThreadGroup() {}
		
		template<typename FUNC>
		void create_thread(FUNC func) {
			std::lock_guard<std::mutex> guard(mutex_);
			threads_.emplace_back(std::thread(func));
		}

		template<typename FUNC>
		void create_thread_count(FUNC func, size_t thread_count) {
			while (thread_count != 0) {
				create_thread(func);
				--thread_count;
			}
		}

		void add_thread(std::thread&& thread) {
			std::lock_guard<std::mutex> guard(mutex_);
			threads_.emplace_back(std::move(thread));
		}

		size_t size() const {
			std::lock_guard<std::mutex> guard(mutex_);
			return threads_.size();
		}

		void join_all() {
			std::lock_guard<std::mutex> guard(mutex_);
			auto itor = threads_.begin();
			while (itor != threads_.end()) {
				itor->join();
				++itor;
			}
		}

		void join_and_clear_all() {
			join_all();
			std::lock_guard<std::mutex> guard(mutex_);
			threads_.clear();
		}

	};

	class ClassReflection {
		std::string type_name_;
	protected:
		ClassReflection() = delete;
		explicit ClassReflection(const std::string& name)
			:type_name_(name) {}
		virtual ~ClassReflection() {}

		std::string type_name() {
			return type_name_;
		}
	};
}