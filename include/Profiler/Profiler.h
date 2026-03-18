#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace profiler
{
	/** Holds the result of a single profiled scope or function.
	 *  @note name must point to storage that outlives the profiling session
	 *        (e.g. a string literal or __FUNCTION__). The profiler does not copy it. */
	struct ProfileResult final
	{
		std::string_view name;
		int64_t start;
		int64_t end;
	};

	/**
	 * Creates parent directories for filepath and removes any existing file at that path.
	 * @param filepath	The target file path to prepare.
	 */
	void PrepareOutputPath(std::string_view filepath);

	/**
	 * CRTP base class for all profiler backends. Provides session management,
	 * frame counting, and Tick() logic. Derived classes must implement:
	 *   void EndSession()
	 *   void WriteProfile(ProfileResult const& result, bool isFunction)
	 *   std::string FlushToString() const
	 * And may optionally shadow:
	 *   void SetThreadName(std::string_view name)
	 *   void MarkFrame(std::string_view name)
	 */
	template<typename Derived>
	class Profiler
	{
	public:
		using FlushCallback = std::function<void(std::string const&)>;

		void BeginSession(std::string const& /*name*/, std::string_view filepath = {}, uint32_t maxFrames = 0, FlushCallback callback = nullptr)
		{
			self().EndSession();
			m_FileName = filepath;
			m_MaxFrames = maxFrames;
			m_ProfiledFrames = 0;
			m_FlushCallback = std::move(callback);
		}

		void SetThreadName(std::string_view /*name*/) noexcept {}
		void MarkFrame(std::string_view /*name*/) noexcept {}

		void Tick()
		{
			if (m_MaxFrames == 0)
			{
				return;
			}

			++m_ProfiledFrames;

			if (m_ProfiledFrames >= m_MaxFrames)
			{
				if (m_FlushCallback)
				{
					m_FlushCallback(self().FlushToString());
					m_FlushCallback = nullptr;
				}

				self().EndSession();
			}
		}

		Profiler(Profiler const&) = delete;
		Profiler(Profiler&&) = delete;
		Profiler& operator=(Profiler const&) = delete;
		Profiler& operator=(Profiler&&) = delete;

	protected:
		Profiler() = default;
		~Profiler() = default;
		[[nodiscard]] std::string const& GetFileName() const noexcept { return m_FileName; }

	private:
		[[nodiscard]] Derived& self() noexcept { return static_cast<Derived&>(*this); }
		[[nodiscard]] Derived const& self() const noexcept { return static_cast<Derived const&>(*this); }

		std::string m_FileName;
		uint32_t m_ProfiledFrames{ 0 };
		uint32_t m_MaxFrames{ 0 };
		FlushCallback m_FlushCallback;
	};
}

#endif
