#include "badge_heartbeat.h"
#include "NetworkTaskManager.h"

int BadgeHeartbeat::status = 0;
extern Scheduler baseTaskRunner;
Scheduler& BadgeHeartbeat::taskRunner = baseTaskRunner;

// Prints free memory
void BadgeHeartbeat::print_free_mem() const {
    Serial.print("Free memory: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
}

// Static wrapper for callback
void BadgeHeartbeat::taskCallback() {
    NetworkTaskManager::getInstance().queueReceivedEventTask("heartbeat", String(ESP.getFreeHeap()));
}

// Create the task
void BadgeHeartbeat::startHeartbeat() {
    Task* heartbeatTask = new Task(60000, TASK_FOREVER, &BadgeHeartbeat::taskCallback, &taskRunner, true, nullptr, nullptr, true);
}

// Stop the task
void BadgeHeartbeat::stopHeartbeat() {
    Serial.println("Stopping heartbeat");
}
