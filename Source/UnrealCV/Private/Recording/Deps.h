// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class Deps
{
public:
	Deps();
	~Deps();
};


#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>



typedef uint64_t TTimePoint;
typedef double TTimeDelta;
/*

class ClockBase {
public:
//returns value indicating nanoseconds elapsed since some reference timepoint in history
//typically nanoseconds from Unix epoch
virtual TTimePoint nowNanos() const = 0;

TTimeDelta elapsedSince(TTimePoint since) const
{
return elapsedBetween(nowNanos(), since);
}
static TTimeDelta elapsedBetween(TTimePoint second, TTimePoint first)
{
return (second - first) / 1.0E9;
}
TTimePoint addTo(TTimePoint t, TTimeDelta dt)
{
return static_cast<TTimePoint>(t + dt * 1.0E9);
}
TTimeDelta updateSince(TTimePoint& since) const
{
TTimePoint cur = nowNanos();
TTimeDelta elapsed = elapsedBetween(cur, since);
since = cur;
return elapsed;
}

virtual TTimePoint step()
{
//by default step doesn't do anything
//for steppeble clock, this would advance to next tick
//for wall clocks, step() is no-op
++step_count_;

return nowNanos();
}

uint64_t getStepCount() const
{
return step_count_;
}

virtual void sleep_for(TTimeDelta dt)
{
if (dt <= 0)
return;

static constexpr std::chrono::duration<double> MinSleepDuration(0);
TTimePoint start = nowNanos();
//spin wait
while (elapsedSince(start) < dt)
std::this_thread::sleep_for(MinSleepDuration);
}

private:
template <typename T>
using duration = std::chrono::duration<T>;

uint64_t step_count_ = 0;
};


class ScalableClock : public ClockBase {
public:
//scale > 1 slows down the clock, < 1 speeds up the clock
ScalableClock(double scale = 1, TTimeDelta latency = 0)
: scale_(scale), latency_(latency)
{
offset_ = latency * (scale_ - 1);
unused(latency_);
}

virtual TTimePoint nowNanos() const override
{
if (offset_ == 0 && scale_ == 1) //optimized normal route
return Utils::getTimeSinceEpochNanos();
else {
/ *
Apply scaling and latency.

Time point is nanoseconds from some reference r. If latency = 0 then r = 0 .
scaled time point is then given by (r + ((now - r) / scale)).
This becomes (r*(s-1) + now)/scale or (offset + now / scale).
* /
return static_cast<TTimePoint>((Utils::getTimeSinceEpochNanos() + offset_) / scale_);
}
}

virtual void sleep_for(TTimeDelta dt) override
{
//for intervals > 2ms just sleep otherwise do spilling otherwise delay won't be accurate
if (dt > 2.0 / 1000) {
TTimeDelta dt_scaled = fromWallDelta(dt);
std::this_thread::sleep_for(std::chrono::duration<double>(dt_scaled));
}
else
ClockBase::sleep_for(dt);
}

protected:
//converts time interval for wall clock to current clock
//For example, if implementation is scaled clock simulating 5X spped then below
//will retun dt*5. This functions are required to translate time to operating system
//which only has concept of wall clock. For example, to make thread sleep for specific duration.

//wall clock to sim clock
virtual TTimeDelta fromWallDelta(TTimeDelta dt) const
{
return dt * scale_;
}
//sim clock to wall clock
virtual TTimeDelta toWallDelta(TTimeDelta dt) const
{
return dt / scale_;
}


private:
typedef std::chrono::high_resolution_clock clock;

double scale_;
TTimeDelta latency_;
double offset_;
};


class ClockFactory {
public:
//output of this function should not be stored as pointer might change
static ClockBase* get(std::shared_ptr<ClockBase> val = nullptr)
{
static std::shared_ptr<ClockBase> clock;

if (val != nullptr)
clock = val;

if (clock == nullptr)
clock = std::make_shared<ScalableClock>();

return clock.get();
}

//don't allow multiple instances of this class
ClockFactory(ClockFactory const&) = delete;
void operator=(ClockFactory const&) = delete;

private:
//disallow instance creation
ClockFactory() {}
};




class CancelableBase {
protected:
std::atomic<bool> is_cancelled_;
std::atomic<bool> is_complete_;
public:
CancelableBase() : is_cancelled_(false), is_complete_(false) {
}

void reset()
{
is_cancelled_ = false;
is_complete_ = false;
}

bool isCancelled() {
return is_cancelled_;
}

void cancel() {
is_cancelled_ = true;
}

virtual void execute() = 0;

virtual bool sleep(double secs)
{
//We can pass duration directly to sleep_for however it is known that on
//some systems, sleep_for makes system call anyway even if passed duration
//is <= 0. This can cause 50us of delay due to context switch.
if (isCancelled()) {
return false;
}

TTimePoint start = ClockFactory::get()->nowNanos();
static constexpr std::chrono::duration<double> MinSleepDuration(0);

while (secs > 0 && !isCancelled() && !is_complete_ &&
ClockFactory::get()->elapsedSince(start) < secs) {

std::this_thread::sleep_for(MinSleepDuration);
}

return !isCancelled();
}

void complete() {
is_complete_ = true;
}

bool isComplete() {
return is_complete_;
}

};*/

// This wraps a condition_variable so we can handle the case where we may signal before wait
// and implement the semantics that say wait should be a noop in that case.
class WorkerThreadSignal
{
	std::condition_variable cv_;
	std::mutex mutex_;
	std::atomic<bool> signaled_;
public:
	WorkerThreadSignal() : signaled_(false)
	{
	}

	void signal()
	{
		{
			std::unique_lock<std::mutex> lock(mutex_);
			signaled_ = true;
		}
		cv_.notify_one();
	}

	template<class _Predicate>
	void wait(_Predicate cancel)
	{
		// wait for signal or cancel predicate
		while (!signaled_) {
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait_for(lock, std::chrono::milliseconds(1), [this, cancel] {
				return cancel();
			});
		}
		signaled_ = false;
	}

	bool waitFor(double max_wait_seconds)
	{
		// wait for signal or timeout or cancel predicate
		while (!signaled_) {
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait_for(lock, std::chrono::milliseconds(
				static_cast<long long>(max_wait_seconds * 1000)));
		}
		signaled_ = false;
		return true;
	}

};
/*

// This class provides a synchronized worker thread that guarantees to execute
// cancelable tasks on a background thread one at a time.
// It guarantees that previous task is cancelled before new task is started.
// The queue size is 1, which means it does NOT guarantee all queued tasks are executed.
// If enqueue is called very quickly the thread will not have a chance to execute each
// task before they get cancelled, worst case in a tight loop all tasks are starved and
// nothing executes.
class WorkerThread {
public:
WorkerThread()
: thread_running_(false), cancel_request_(false) {
}

~WorkerThread() {
cancel_request_ = true;
cancel();
}
void enqueue(std::shared_ptr<CancelableBase> item) {
//cancel previous item
{
std::unique_lock<std::mutex> lock(mutex_);
std::shared_ptr<CancelableBase> pending = pending_item_;
if (pending != nullptr) {
pending->cancel();
}
}

bool running = false;

{
std::unique_lock<std::mutex> lock(mutex_);
pending_item_ = item;
running = thread_running_;
}

if (running) {
item_arrived_.signal();
}
else {
start();
}
}

bool enqueueAndWait(std::shared_ptr<CancelableBase> item, float max_wait_seconds)
{
//cancel previous item
{
std::unique_lock<std::mutex> lock(mutex_);
std::shared_ptr<CancelableBase> pending = pending_item_;
if (pending != nullptr) {
pending->cancel();
}
}

bool running = false;

//set new item to run
{
std::unique_lock<std::mutex> lock(mutex_);
pending_item_ = item;
running = thread_running_;
}

if (running) {
item_arrived_.signal();
}
else {
start();
}

item->sleep(max_wait_seconds);

//after the wait if item is still running then cancel it
if (!item->isCancelled() && !item->isComplete())
item->cancel();

return !item->isCancelled();
}

void cancel()
{
std::unique_lock<std::mutex> lock(mutex_);
std::shared_ptr<CancelableBase> pending = pending_item_;
pending_item_ = nullptr;
if (pending != nullptr) {
pending->cancel();
}
if (thread_.joinable()) {
item_arrived_.signal();
thread_.join();
}
}
private:
void start()
{
//if state == not running
if (!thread_running_) {

//make sure C++ previous thread is done
{
std::unique_lock<std::mutex> lock(mutex_);
if (thread_.joinable()) {
thread_.join();
}
}
Utils::cleanupThread(thread_);

//start the thread
cancel_request_ = false;
thread_ = std::thread(&WorkerThread::run, this);

//wait until thread tells us it has started
thread_started_.wait([this] { return static_cast<bool>(cancel_request_); });
}
}

void run()
{
thread_running_ = true;

//tell the thread which started this thread that we are on now
{
std::unique_lock<std::mutex> lock(mutex_);
thread_started_.signal();
}

//until we don't get stopped and have work to do, keep running
while (!cancel_request_ && pending_item_ != nullptr) {
std::shared_ptr<CancelableBase> pending;

//get the pending item
{
std::unique_lock<std::mutex> lock(mutex_);
pending = pending_item_;
}

//if pending item is not yet cancelled
if (pending != nullptr && !pending->isCancelled()) {

//execute pending item
try {
pending->execute();
}
catch (std::exception& e) {
//Utils::DebugBreak();
Utils::log(Utils::stringf("WorkerThread caught unhandled exception: %s", e.what()), Utils::kLogLevelError);
}

//we use cancel here to communicate to enqueueAndWait that the task is complete.
pending->complete();
}

if (!cancel_request_) {
//wait for next item to arrive or thread is stopped
item_arrived_.wait([this] { return static_cast<bool>(cancel_request_); });
}
}

thread_running_ = false;
}

private:
//this is used to wait until our thread actually gets started
WorkerThreadSignal thread_started_;
//when new item arrived, we signal this so waiting thread can continue
WorkerThreadSignal item_arrived_;

// thread state
std::shared_ptr<CancelableBase> pending_item_;
std::mutex mutex_;
std::thread thread_;
//while run() is in progress this is true
std::atomic<bool> thread_running_;
//has request to stop this worker thread made?
std::atomic<bool> cancel_request_;
};
*/