#include "Leader.h"

Leader::Leader(unsigned term) {
    currentTerm_ = term;
    SetRole(NodeRole::Leader);
}

// TODO: delete this
bool Leader::WriteLog() {
    // sendMessage to followers -> wait answer
    // -> if all of them ok -> write to myself
    //          if no -> rollback
    return false;
}

void Leader::HandleVoteRequest(RContext r_context, OContext &o_context) {

}

void Leader::HandleHeartBeat(RContext r_context, OContext &o_context) {

}

void Leader::HandleVoteResponse(RContext r_context, OContext &o_context) {

}

void Leader::HandleElectionTimeout(RContext r_context, OContext &o_context) {

}

//TODO: должны включить commitIndex. Это сообщает подписчикам, что они должны зафиксировать (commit) и применить все записи до этого индекса к своему log (состоянию)
// если он больше или равено, но не засинкан (решить нужно ли нам доп состояние sync)
void Leader::SendHeartBeat(RContext r_context, OContext &o_context) {
    std::string heartbeat = "HeartBeat receivedTerm=" + std::to_string(currentTerm_) + " RAVE RAVE HEARTBEAT" + "\n";

    o_context.send_msg(heartbeat);
    o_context.notifyAll = true;
    auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM / 3, TIMEOUT_TO / 3)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));
    std::cout << "Heartbeat sent.." << "\n";
}

void Leader::ReceiveDataFromClient(RContext r_context, OContext &o_context) {
    auto data = ExtractDataFromClientMessage(r_context.message.message);
    log.emplace_back(std::move(data), currentTerm_);
    // ++commitIndex;

    // Не уверен
    r_context.message.message = data;
    SendAppendEntries(r_context, o_context);
}

// После ответа всех челиков из peers, если большая часть ответила - обновляем commitIndex
void Leader::UpdateCommitIndex() {
    // После чего применяем запись и
}

void Leader::SendAppendEntries(const RContext& r_context, OContext &o_context) {
    // Формирование запроса AppendEntries:
    // - currentTerm
    // - prevLogIndex и prevLogTerm: чтобы определить корректность предыдущей записи
    // - entries: новые записи (если есть)
    // - leaderCommit: commit index

    auto prevLogIndex = commitIndex == 0 ? 0 : commitIndex -1;
    auto prevLogTerm = currentTerm_ == 1 ? 1 : currentTerm_ - 1;




    std::string appendEntries = "AppendEntries term=" + std::to_string(currentTerm_) +
                                " prevLogIndex=" + std::to_string(prevLogIndex) +
                                " prevLogTerm=" + std::to_string(prevLogTerm) +
                                " entries=" + r_context.message.message +
                                " leaderCommit=" + std::to_string(commitIndex) + "\n";

    o_context.notifyAll = true;
    o_context.message = appendEntries;
    //TODO: o_context.next_time_out  <=== do we need timer here?
}




std::string Leader::ExtractDataFromClientMessage(const std::string &message) {
    size_t pos = message.find("Client:");
    size_t posAfter = pos + std::string("Client:").size();
    return message.substr(posAfter);
}

void Leader::HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) {
    unsigned int responseTerm = ExtractTermFromMessage(r_context.message.message);

    // Если term ответа больше чем у лидера - должен перейти в режим Follower
    if (responseTerm > currentTerm_) {
        std::cout << "Received higher term in AppendEntries response ("
                  << responseTerm << "), stepping down." << "\n";
        SetRole(NodeRole::Follower);
        currentTerm_ = responseTerm;
        //TODO: Дополнительные действия при переходе Follower (сброс таймеров, каст)
        //
        return;
    }

    // Предположим, что ответ содержит строку "AppendSuccess" или "AppendFailure"
    bool success = (r_context.message.message.find("AppendSuccess") != std::string::npos);

    if (success) {
        std::cout << "AppendEntries succeeded for follower "
                  << r_context.message.sender.value() << "\n";
        //TODO: Обновляем индексы для этого Follower (например, nextIndex и matchIndex или чет такого рода)
        // Если подтверждения получены от большинства, обновляем commitIndex и применяем запись.


    } else {
        std::cout << "AppendEntries failed for follower "
                  << r_context.message.sender.value() << ", retrying..." << "\n";
        //TODO: Уменьшаем индекс для данного подписчика и повторяем попытку отправки AppendEntries.
        // Повторно вызываем отправку AppendEntries для этого подписчика.

    }
}

void Leader::HandleAppendEntries(RContext r_context, OContext &o_context) {

}
