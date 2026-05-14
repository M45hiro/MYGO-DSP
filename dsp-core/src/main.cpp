#include "Engine.h"
#include "AutoFilterDesigner.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>

// Minimal JSON parser/writer for IPC over stdin/stdout
// Protocol: one JSON command per line, one JSON response per line

static std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

static void writeLine(const std::string& s) {
    std::cout << s << "\n";
    std::cout.flush();
}

// Simple JSON escape
static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += c;
        }
    }
    return out;
}

// Extract a quoted string value from JSON
static std::string extractString(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return {};
    pos = json.find(':', pos);
    if (pos == std::string::npos) return {};
    pos = json.find_first_of("\"0123456789-", pos);
    if (pos == std::string::npos) return {};
    if (json[pos] == '"') {
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return {};
        return json.substr(pos + 1, end - pos - 1);
    }
    return {};
}

static double extractNumber(const std::string& json, const std::string& key, double def = 0.0) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return def;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return def;
    pos = json.find_first_of("0123456789-", pos);
    if (pos == std::string::npos) return def;
    size_t end = json.find_first_of(",}\n]", pos);
    if (end == std::string::npos) end = json.size();
    try { return std::stod(json.substr(pos, end - pos)); }
    catch (...) { return def; }
}

static int extractInt(const std::string& json, const std::string& key, int def = 0) {
    return static_cast<int>(extractNumber(json, key, def));
}

static std::string response(const std::string& id, const std::string& status,
                            const std::string& data = "null") {
    // If id is purely numeric, emit as number; otherwise as string
    bool isNum = !id.empty() && id.find_first_not_of("0123456789") == std::string::npos;
    if (isNum)
        return "{\"id\":" + id + ",\"status\":\"" + status + "\",\"data\":" + data + "}";
    else
        return "{\"id\":\"" + jsonEscape(id) + "\",\"status\":\"" + status + "\",\"data\":" + data + "}";
}

int main() {
    auto& engine = mygo_dsp::Engine::instance();
    engine.init(44100.0, 512);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::string cmd = extractString(line, "cmd");
        std::string reqId = extractString(line, "id");
        if (reqId.empty()) {
            // Try numeric id
            double numId = extractNumber(line, "id", -1);
            if (numId >= 0) reqId = std::to_string(static_cast<int>(numId));
            else reqId = cmd;
        }

        if (cmd == "exit") break;

        else if (cmd == "init") {
            double sr = extractNumber(line, "sampleRate", 44100.0);
            int bs = extractInt(line, "bufferSize", 512);
            engine.init(sr, bs);
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "addWave") {
            mygo_dsp::WaveformParams p;
            p.type = static_cast<mygo_dsp::WaveformType>(extractInt(line, "type", 0));
            p.frequency = extractNumber(line, "frequency", 440.0);
            p.amplitude = extractNumber(line, "amplitude", 0.5);
            p.phase = extractNumber(line, "phase", 0.0);
            p.dutyCycle = extractNumber(line, "dutyCycle", 0.5);
            p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
            int id = engine.addWave(p);
            writeLine(response(reqId, "ok", std::to_string(id)));
        }

        else if (cmd == "removeWave") {
            int id = extractInt(line, "wid");
            engine.removeWave(id);
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "updateWave") {
            int id = extractInt(line, "wid");
            mygo_dsp::WaveformParams p;
            p.type = static_cast<mygo_dsp::WaveformType>(extractInt(line, "type", 0));
            p.frequency = extractNumber(line, "frequency", 440.0);
            p.amplitude = extractNumber(line, "amplitude", 0.5);
            p.phase = extractNumber(line, "phase", 0.0);
            p.dutyCycle = extractNumber(line, "dutyCycle", 0.5);
            p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
            engine.updateWave(id, p);
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "addFilterFIR") {
            mygo_dsp::FIRFilterParams p;
            p.order = extractInt(line, "order", 64);
            p.window = static_cast<mygo_dsp::WindowType>(extractInt(line, "window", 1));
            p.cutoff = extractNumber(line, "cutoff", 1000.0);
            p.cutoff2 = extractNumber(line, "cutoff2", 4000.0);
            p.passType = static_cast<mygo_dsp::FilterPassType>(extractInt(line, "passType", 0));
            p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
            int id = engine.addFilterFIR(p);
            writeLine(response(reqId, "ok", std::to_string(id)));
        }

        else if (cmd == "addFilterIIR") {
            mygo_dsp::IIRFilterParams p;
            p.prototype = static_cast<mygo_dsp::IIRPrototype>(extractInt(line, "prototype", 0));
            p.passType = static_cast<mygo_dsp::IIRPassType>(extractInt(line, "passType", 0));
            p.order = extractInt(line, "order", 4);
            p.cutoff = extractNumber(line, "cutoff", 1000.0);
            p.cutoff2 = extractNumber(line, "cutoff2", 4000.0);
            p.ripple = extractNumber(line, "ripple", 0.5);
            p.stopbandAtten = extractNumber(line, "stopbandAtten", 40.0);
            p.shelfGain = extractNumber(line, "shelfGain", 1.0);
            p.Q = extractNumber(line, "Q", 0.707);
            p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
            int id = engine.addFilterIIR(p);
            writeLine(response(reqId, "ok", std::to_string(id)));
        }

        else if (cmd == "removeFilter") {
            int id = extractInt(line, "fid");
            engine.removeFilter(id);
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "updateFilter") {
            int id = extractInt(line, "fid");
            int isIIRorFIR = extractInt(line, "isIIR", 0);
            if (isIIRorFIR == 1) {
                mygo_dsp::IIRFilterParams p;
                p.prototype = static_cast<mygo_dsp::IIRPrototype>(extractInt(line, "prototype", 0));
                p.passType = static_cast<mygo_dsp::IIRPassType>(extractInt(line, "passType", 0));
                p.order = extractInt(line, "order", 4);
                p.cutoff = extractNumber(line, "cutoff", 1000.0);
                p.cutoff2 = extractNumber(line, "cutoff2", 4000.0);
                p.ripple = extractNumber(line, "ripple", 0.5);
                p.stopbandAtten = extractNumber(line, "stopbandAtten", 40.0);
                p.shelfGain = extractNumber(line, "shelfGain", 1.0);
                p.Q = extractNumber(line, "Q", 0.707);
                p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
                engine.updateFilter(id, p);
            } else {
                mygo_dsp::FIRFilterParams p;
                p.order = extractInt(line, "order", 64);
                p.window = static_cast<mygo_dsp::WindowType>(extractInt(line, "window", 1));
                p.cutoff = extractNumber(line, "cutoff", 1000.0);
                p.cutoff2 = extractNumber(line, "cutoff2", 4000.0);
                p.passType = static_cast<mygo_dsp::FilterPassType>(extractInt(line, "passType", 0));
                p.sampleRate = extractNumber(line, "sampleRate", 44100.0);
                engine.updateFilter(id, p);
            }
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "setBypass") {
            int id = extractInt(line, "bid");
            bool bypass = extractInt(line, "bypass", 0) != 0;
            std::string target = extractString(line, "target");
            if (target == "wave")
                engine.setWaveBypass(id, bypass);
            else
                engine.setFilterBypass(id, bypass);
            writeLine(response(reqId, "ok"));
        }

        else if (cmd == "process") {
            const auto& data = engine.processAudio();
            std::string arr = "[";
            char buf[32];
            size_t n = std::min(data.size(), size_t(128));
            for (size_t i = 0; i < n; ++i) {
                if (i > 0) arr += ",";
                snprintf(buf, sizeof(buf), "%.4g", data[i]);
                arr += buf;
            }
            arr += "]";
            writeLine(response(reqId, "ok", arr));
        }

        else if (cmd == "autoDesign") {
            mygo_dsp::FilterSpec spec;
            spec.type = static_cast<mygo_dsp::FilterSpec::Type>(extractInt(line, "type", 0));
            spec.fs = extractNumber(line, "fs", 44100.0);
            spec.fpass = extractNumber(line, "fpass", 1000.0);
            spec.fstop = extractNumber(line, "fstop", 2000.0);
            spec.fpass2 = extractNumber(line, "fpass2", 0.0);
            spec.fstop2 = extractNumber(line, "fstop2", 0.0);
            spec.Ap = extractNumber(line, "Ap", 1.0);
            spec.As = extractNumber(line, "As", 40.0);

            auto res = mygo_dsp::AutoFilterDesigner::autoDesign(spec);

            std::string json = "{";
            json += "\"success\":" + std::string(res.success ? "true" : "false") + ",";
            json += "\"warning\":\"" + jsonEscape(res.warning) + "\",";
            json += "\"useFIR\":" + std::string(res.useFIR ? "true" : "false") + ",";
            json += "\"selectedProto\":" + std::to_string(static_cast<int>(res.selectedProto)) + ",";
            json += "\"order\":" + std::to_string(res.order) + ",";
            json += "\"firOrder\":" + std::to_string(res.firOrder) + ",";
            json += "\"firBeta\":" + std::to_string(res.firBeta);
            json += "}";
            writeLine(response(reqId, "ok", json));
        }

        else if (cmd == "getCounts") {
            std::string data = "{\"waves\":" + std::to_string(engine.getWaveCount()) +
                               ",\"filters\":" + std::to_string(engine.getFilterCount()) + "}";
            writeLine(response(reqId, "ok", data));
        }

        else if (cmd == "getPoleZero") {
            int fid = extractInt(line, "fid");
            std::vector<std::complex<double>> poles, zeros;
            auto fmtComplex = [](const std::complex<double>& c) {
                return "{\"real\":" + std::to_string(c.real()) + ",\"imag\":" + std::to_string(c.imag()) + "}";
            };
            auto arrToJson = [&](const std::vector<std::complex<double>>& v) {
                std::string s = "[";
                for (size_t i = 0; i < v.size(); ++i) {
                    if (i > 0) s += ",";
                    s += fmtComplex(v[i]);
                }
                return s + "]";
            };
            if (engine.getFilterPoleZero(fid, poles, zeros)) {
                writeLine(response(reqId, "ok", "{\"poles\":" + arrToJson(poles) + ",\"zeros\":" + arrToJson(zeros) + "}"));
            } else if (engine.getFilterZeros(fid, zeros)) {
                writeLine(response(reqId, "ok", "{\"poles\":[],\"zeros\":" + arrToJson(zeros) + "}"));
            } else {
                writeLine(response(reqId, "ok", "{\"poles\":[],\"zeros\":[]}"));
            }
        }

        else {
            writeLine(response(reqId, "error", "\"unknown command\""));
        }
    }

    return 0;
}
