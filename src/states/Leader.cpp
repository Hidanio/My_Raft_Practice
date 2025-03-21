#include "Leader.h"

Leader::Leader(unsigned term) {
    currentTerm_ = term;
    SetRole(NodeRole::Leader);
    commitIndex = 0;
    lastApplied = 0;
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
    // if leader handle heartbeat with higher term => cast to follower
    unsigned int responseTerm = ExtractTermFromMessage(r_context.message.message);
    if (responseTerm > currentTerm_) {
        std::cout << "Received higher term heartbeat (" << responseTerm << "), stepping down." << "\n";
        SetRole(NodeRole::Follower);
        currentTerm_ = responseTerm;

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        std::swap(r_context.node_, base_ptr);
    }
}

void Leader::HandleVoteResponse(RContext r_context, OContext &o_context) {

}

void Leader::HandleElectionTimeout(RContext r_context, OContext &o_context) {

}

//TODO: должны включить commitIndex. Это сообщает подписчикам, что они должны зафиксировать (commit) и применить все записи до этого индекса к своему log (состоянию)
// если он больше или равено, но не засинкан (решить нужно ли нам доп состояние sync)
void Leader::SendHeartBeat(RContext r_context, OContext &o_context) {
    std::string heartbeat = "HeartBeat receivedTerm=" + std::to_string(currentTerm_) +
                            " leaderCommit=" + std::to_string(commitIndex) +
                            " RAVE RAVE HEARTBEAT" + "\n";
    o_context.send_msg(heartbeat);
    o_context.notifyAll = true;
    auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM / 3, TIMEOUT_TO / 3)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));
    std::cout << "Heartbeat sent.." << "\n";
}

void Leader::ReceiveDataFromClient(RContext r_context, OContext &o_context) {
    auto data = ExtractDataFromClientMessage(r_context.message.message);
    log.emplace_back(std::move(data), currentTerm_);

    std::cout << "Leader appended client command in term " << currentTerm_ << "\n";

    // Не уверен
    r_context.message.message = data;
    SendAppendEntries(r_context, o_context);
}

// После ответа всех челиков из peers, если большая часть ответила - обновляем commitIndex
// и в heartbeat фолловеры получают leaderCommit в heartbeat и обновляют свой commitIndex до этого значения (если их commitIndex меньше)
void Leader::UpdateCommitIndex() {
    // Ищем такой индекс N (от commitIndex+1 до log.size()), что N больше текущего commitIndex,
    // и более половины подписчиков (включая лидера) имеют matchIndex >= N, и запись N находится в текущем term.
    for (unsigned n = commitIndex + 1; n <= log.size(); n++) {
        unsigned count = 1; // лидер всегда имеет эту запись
        for (const auto &kv : matchIndex_) {
            if (kv.second >= n) {
                count++;
            }
        }
        // Если подтверждений от большинства получено:
        if (count > (matchIndex_.size() + 1) / 2) {
            // Только если запись в log[n-1] принадлежит текущему term
            if (std::get<1>(log[n - 1]) == currentTerm_) {
                commitIndex = n;
                std::cout << "Updated commitIndex to " << commitIndex << "\n";
            }
        }
    }
    ApplyLogEntries();
}

void Leader::SendAppendEntries(const RContext &r_context, OContext &o_context) {
    // Формирование запроса AppendEntries:
    // - currentTerm
    // - prevLogIndex и prevLogTerm: чтобы определить корректность предыдущей записи
    // - entries: новые записи (если есть)
    // - leaderCommit: commit index

    if (log.empty()) return;

    unsigned prevLogIndex = (commitIndex == 0) ? 0 : commitIndex;
    unsigned prevLogTerm = 0;

    if (prevLogIndex > 0 && prevLogIndex <= log.size()) {
        prevLogTerm = std::get<1>(log[prevLogIndex - 1]);  // лог с 1 индексируется ??
    }

    // Собираем новые записи для репликации: отправляем запись с индексом commitIndex+1, если она есть
    std::string entries;
    if (commitIndex < log.size()) {
        entries = std::get<0>(log[commitIndex]);  // отправляем следующую запись
    }

    std::string appendEntries = "AppendEntries term=" + std::to_string(currentTerm_) +
                                " prevLogIndex=" + std::to_string(prevLogIndex) +
                                " prevLogTerm=" + std::to_string(prevLogTerm) +
                                " entries=" + entries +
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
    auto followerEndpoint = r_context.message.sender.value();

    // Если term ответа больше чем у лидера - должен перейти в режим Follower
    if (responseTerm > currentTerm_) {
        std::cout << "Received higher term in AppendEntries response ("
                  << responseTerm << "), stepping down." << "\n";
        SetRole(NodeRole::Follower);
        currentTerm_ = responseTerm;

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        std::swap(r_context.node_, base_ptr);
        //TODO: Дополнительные действия при переходе Follower (сброс таймеров, каст)
        //
        return;
    }

    // Предположим, что ответ содержит строку "AppendSuccess" или "AppendFailure"
    bool success = (r_context.message.message.find("AppendSuccess") != std::string::npos);

    if (success) {
        std::cout << "AppendEntries succeeded for follower "
                  << r_context.message.sender.value() << "\n";

        if (nextIndex_.find(followerEndpoint) == nextIndex_.end()) {
            nextIndex_[followerEndpoint] = log.size() + 1; // следующий индекс – размер лога + 1
            matchIndex_[followerEndpoint] = 0;
        }
        //TODO: Обновляем индексы для этого Follower (например, nextIndex и matchIndex или чет такого рода)
        // Если подтверждения получены от большинства, обновляем commitIndex и применяем запись.

        // При успешном AppendEntries предполагаем, что подписчик получил записи до nextIndex
        nextIndex_[followerEndpoint] = log.size() + 1;
        matchIndex_[followerEndpoint] = log.size();
        UpdateCommitIndex();

    } else {
        std::cout << "AppendEntries failed for follower " << followerEndpoint << ", retrying..." << "\n";

        //TODO: Уменьшаем индекс для данного подписчика и повторяем попытку отправки AppendEntries.
        // Повторно вызываем отправку AppendEntries для этого подписчика.
        // Если AppendEntries вымер, то уменьшаем nextIndex и повторяем попытку
        if (nextIndex_[followerEndpoint] > 1) {
            nextIndex_[followerEndpoint] -= 1;
        }

        // вот тут повторная отправка, но нужно модифицировать для конкретного подписчика!!! (на подумать)
        SendAppendEntries(r_context,o_context);
    }
}

void Leader::HandleAppendEntries(RContext r_context, OContext &o_context) {

}

void Leader::ApplyLogEntries() {
    while (lastApplied < commitIndex) {
        lastApplied++;
        std::string command = std::get<0>(log[lastApplied - 1]);
        std::cout << "Applying command: " << command << " at index " << lastApplied << "\n";
        // Здесь вызывается бизнес-логика для применения команды к состоянию (например, изменение данных), но для прототипа нам не нужно
    }
}
