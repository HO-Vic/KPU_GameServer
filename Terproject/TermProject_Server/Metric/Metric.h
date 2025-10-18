#pragma once
#include <cstdint>
#include <atomic>
#include <mutex>

class Metric{
	struct MetricSlot{
		std::atomic_uint64_t totalGridElapsedTime = 0;//�׸��� ���� ��Ŀ��� ���� �ð�
		std::atomic_uint32_t totalProcessCnt = 0;//�׸��� ���� �ڵ带 ��� ������
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
	//�̱����̱� ������, ���� �̵� xx
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
