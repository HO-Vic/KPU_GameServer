#pragma once
#include <cstdint>
#include <atomic>
#include <mutex>

class Metric{
	struct MetricSlot{
		std::atomic_uint64_t totalGridElapsedTime = 0;//그리드 분할 방식에서 사용된 시간
		std::atomic_uint32_t totalProcessCnt = 0;//그리드 분할 코드를 몇번 탔는지
		MetricSlot() = default;

		void Clear(){
			totalGridElapsedTime = 0;
			totalProcessCnt = 0;
		}
	};

public:
	static Metric & GetInstance(){
		static Metric metric;
		return metric;
	}
private:
	Metric() = default;
public:
	//싱글톤이기 떄문에, 복사 이동 xx
	Metric(const Metric &) = delete;
	Metric(Metric &&) = delete;

	MetricSlot & SwapAndLoad(){
		int prev = 0;
		{
			std::lock_guard<std::mutex>lg(m_lock);
			prev = currentIdx;
			currentIdx = ( currentIdx + 1 ) % 2;
		}
		return m_metrics[prev];
	}

private:
	std::mutex m_lock;
	MetricSlot m_metrics[2];
	int currentIdx = 0;
};
