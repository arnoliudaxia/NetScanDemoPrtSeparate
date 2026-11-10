"""Hardware-free tests of the real GBK MFC encoder reservation handlers.

Run: python -m unittest discover -s NetScanDemoPrtSeparate/tests -v

Only the named declarations/methods are extracted from NetScanDemoDlg.cpp;
no application headers, vendor libraries, GUI, socket API, or devices are used.
Windows SDK types/interlocked operation are real; controls, logging, system-info
and all device/encoder SDK functions are fakes. This tests handler results, not the
socket parser/reply transport or MFC message dispatch. Missing production methods
are an error (not a skip or a fallback to the .netscan-edit working copy).
"""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


PROJECT = Path(__file__).resolve().parents[1]
SOURCE = PROJECT / "NetScanDemo" / "NetScanDemoDlg.cpp"
VSDEVCMD = Path(
    "C:/Program Files/Microsoft Visual Studio/18/Community/Common7/Tools/VsDevCmd.bat"
)


def _extract_block(text, pattern, *, semicolon=False):
    """Extract exactly one brace-balanced definition, ignoring comments/literals."""
    matches = list(re.finditer(pattern, text, re.MULTILINE))
    if len(matches) != 1:
        raise AssertionError(
            f"Expected one definition matching {pattern!r} in {SOURCE}, "
            f"found {len(matches)}. Has the GBK production source been synchronized?"
        )
    start = matches[0].start()
    opening = text.index("{", matches[0].end())
    # A raw-string literal is not currently used, but support it to avoid counting
    # braces in future diagnostic messages. Comments must precede the char token.
    tokens = re.compile(
        r'//[^\n]*|/\*[\s\S]*?\*/|R"([^ ()\\\t\r\n]{0,16})\([\s\S]*?\)\1"'
        r'|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'|[{}]'
    )
    depth = 0
    for token in tokens.finditer(text, opening):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if depth == 0:
                end = token.end()
                if semicolon:
                    suffix = re.match(r"\s*;", text[end:])
                    if suffix is None:
                        raise AssertionError(f"Missing declaration semicolon: {pattern}")
                    end += suffix.end()
                line = text.count("\n", 0, start) + 1
                return f'#line {line} "{SOURCE.as_posix()}"\n' + text[start:end]
    raise AssertionError(f"Unbalanced definition: {pattern}")


def _production_fragments():
    text = SOURCE.read_bytes().decode("gbk")
    specs = (
        (r"^\s*enum SocketEncoderAction\s*(?=\{)", True),
        (r"^\s*struct SocketEncoderRequest\s*(?=\{)", True),
        (r"^\s*static const char\* const ENCODER_COMMAND_NAMES\[\]\s*=\s*(?=\{)", True),
        (r"^\s*static int GetSocketEncoderAction\(const std::string& command\)\s*(?=\{)", False),
        (r"^\s*BOOL CNetScanDemoDlg::ExecuteEncoderReset\(\)\s*(?=\{)", False),
        (r"^\s*LRESULT CNetScanDemoDlg::OnSocketEncoderControl\(WPARAM wParam, LPARAM lParam\)\s*(?=\{)", False),
    )
    fragments = [_extract_block(text, p, semicolon=s) for p, s in specs]
    header = (PROJECT / "NetScanDemo" / "stdafx.h").read_bytes().decode("gbk")
    constants = re.findall(r"^\s*#define[ \t]+ZERO_POSITION[ \t]+[^\r\n]+", header, re.MULTILINE)
    if len(constants) != 1:
        raise AssertionError("Expected the real ZERO_POSITION definition in stdafx.h")
    return constants[0] + "\n" + "\n\n".join(fragments[:4]), "\n\n".join(fragments[4:])


PREAMBLE = r'''
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#define _T(x) L##x
using SOCKET = UINT_PTR;
constexpr SOCKET INVALID_SOCKET = static_cast<SOCKET>(-1);
// Identity-only fake control IDs; no resource/MFC/vendor headers are included.
constexpr int IDC_BUTTON_STARTJOB = 1;
constexpr int IDC_BUTTON_RESET_ENCODER = 2;
#define CHECK(x) do { if (!(x)) throw std::runtime_error( \
    std::string("line ") + std::to_string(__LINE__) + ": " #x); } while (0)

// Only the SDK field accessed by the extracted handlers is needed. Cached
// g_pSysInfo and the latest SDK snapshot are intentionally separate objects.
struct PRINTER_INFO { int nIoState[1] = {}; } systemInfo;
using LPPRINTER_INFO = PRINTER_INFO*;
struct FakeSdk {
    bool connected = true;
    bool infoAvailable = true;
    PRINTER_INFO latestInfo;
    int connectionChecks = 0;
    int infoQueries = 0;
    bool succeeds = true;
    int resets = 0;
    int reads = 0;
    unsigned int lastPosition = 0;
    unsigned int readback = 0x12345678;
    int logs = 0;
    std::vector<std::string> trace;
} sdk;

// These local definitions are the only SDK implementation in the executable.
static bool __stdcall DEV_DeviceIsConnected() {
    ++sdk.connectionChecks;
    return sdk.connected;
}
static LPPRINTER_INFO __stdcall DEV_GetDeviceInfo() {
    ++sdk.infoQueries;
    return sdk.infoAvailable ? &sdk.latestInfo : nullptr;
}
static bool __stdcall DEV_ResetPrinterEncValue(unsigned int position) {
    ++sdk.resets;
    sdk.lastPosition = position;
    sdk.trace.push_back("reset");
    return sdk.succeeds;
}
static unsigned int __stdcall DEV_GetPrinterEncValue() {
    ++sdk.reads;
    sdk.trace.push_back("read");
    return sdk.readback;
}
static LPPRINTER_INFO g_pSysInfo = &systemInfo;
class CNetScanDemoDlg;
static void WriteProcessLog(CNetScanDemoDlg*, const wchar_t*, ...) {
    ++sdk.logs;
    sdk.trace.push_back("log");
}
struct FakeButton {
    BOOL enabled = TRUE;
    int changes = 0;
    void EnableWindow(BOOL value) { enabled = value; ++changes; }
};
'''

FAKE_DIALOG = r'''
#line 1 "fake_dialog"
class CNetScanDemoDlg {
public:
    LONG m_nEncoderResetToken = 0;
    SOCKET m_hEncoderResetOwner = INVALID_SOCKET;
    int m_nEncoderResetResult = 0;
    BOOL m_bClosing = FALSE;
    volatile LONG m_lStopSocketServer = 0;
    BOOL m_bJobStarted = FALSE;
    BOOL m_bPrinting = FALSE;
    BOOL m_bCheckingPythonConnection = FALSE;
    void* m_pSocketPressInkRequest = nullptr;
    BOOL m_bAutoPressInkActive = FALSE;
    BOOL m_bDeviceInitialized = TRUE;
    BOOL m_bAirInkDialogOpen = FALSE;
    BOOL m_bPressInkOutputUnknown = FALSE;
    FakeButton start, reset;
    FakeButton* GetDlgItem(int id) {
        CHECK(id == IDC_BUTTON_STARTJOB || id == IDC_BUTTON_RESET_ENCODER);
        return id == IDC_BUTTON_STARTJOB ? &start : &reset;
    }
    BOOL ExecuteEncoderReset();
    LRESULT OnSocketEncoderControl(WPARAM, LPARAM);
};
'''

SCENARIOS = r'''
#line 1 "encoder_test_scenarios"
static void request(CNetScanDemoDlg& dlg, SocketEncoderAction action,
                    const char* expected = nullptr, SOCKET owner = 41, LONG token = 101) {
    SocketEncoderRequest req = { action, owner, token, nullptr };
    CHECK(dlg.OnSocketEncoderControl(0, reinterpret_cast<LPARAM>(&req)) == 0);
    if (expected == nullptr) CHECK(req.pszError == nullptr);
    else CHECK(req.pszError != nullptr && std::strcmp(req.pszError, expected) == 0);
}
static void reserved(const CNetScanDemoDlg& dlg, SOCKET owner = 41, LONG token = 101,
                     int result = 0) {
    CHECK(dlg.m_nEncoderResetToken == token);
    CHECK(dlg.m_hEncoderResetOwner == owner);
    CHECK(dlg.m_nEncoderResetResult == result);
    CHECK(!dlg.start.enabled && !dlg.reset.enabled);
}
static void released(const CNetScanDemoDlg& dlg, bool buttonsEnabled = true) {
    CHECK(dlg.m_nEncoderResetToken == 0);
    CHECK(dlg.m_hEncoderResetOwner == INVALID_SOCKET);
    CHECK(dlg.m_nEncoderResetResult == 0);
    CHECK(!!dlg.start.enabled == buttonsEnabled);
    CHECK(!!dlg.reset.enabled == buttonsEnabled);
}
// Readiness queries are allowed; no encoder reset/read may occur on rejection.
static void noSdk() { CHECK(sdk.resets == 0 && sdk.reads == 0); }
static void clearEnvironment() {
    sdk = FakeSdk{};
    systemInfo = PRINTER_INFO{};
    g_pSysInfo = &systemInfo;
}

static void begin_only() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    reserved(dlg);
    CHECK(dlg.start.changes == 1 && dlg.reset.changes == 1);
    CHECK(sdk.logs == 1);
    noSdk();
}
static void begin_idempotent() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_BEGIN);
    CHECK(dlg.start.changes == 1 && dlg.reset.changes == 1 && sdk.logs == 1);
    reserved(dlg);
    request(dlg, ENCODER_RESET_EXECUTE);
    request(dlg, ENCODER_RESET_BEGIN);
    reserved(dlg, 41, 101, 1); // BEGIN must not discard the cached RESET result.
    request(dlg, ENCODER_RESET_EXECUTE);
    CHECK(sdk.resets == 1 && sdk.reads == 1);
}
static void foreign_reservations() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    const SOCKET owners[] = {42, 41, 42};
    const LONG tokens[] = {101, 102, 102};
    for (int i = 0; i < 3; ++i) {
        request(dlg, ENCODER_RESET_BEGIN, "BUSY", owners[i], tokens[i]);
        request(dlg, ENCODER_RESET_EXECUTE, "NOT_RESERVED", owners[i], tokens[i]);
        request(dlg, ENCODER_RESET_END, "NOT_RESERVED", owners[i], tokens[i]);
        request(dlg, ENCODER_RESET_CANCEL, nullptr, owners[i], tokens[i]);
        reserved(dlg);
    }
    CHECK(dlg.start.changes == 1 && dlg.reset.changes == 1 && sdk.logs == 1);
    noSdk();
}
static void setBusy(CNetScanDemoDlg& dlg, int flag) {
    if (flag == 0) dlg.m_bJobStarted = TRUE;
    if (flag == 1) dlg.m_bPrinting = TRUE;
    if (flag == 2) dlg.m_bCheckingPythonConnection = TRUE;
    if (flag == 3) dlg.m_pSocketPressInkRequest = &systemInfo;
    if (flag == 4) dlg.m_bAutoPressInkActive = TRUE;
}
static void busy_guards() {
    for (int flag = 0; flag < 5; ++flag) {
        clearEnvironment();
        CNetScanDemoDlg idle;
        setBusy(idle, flag);
        request(idle, ENCODER_RESET_BEGIN, "BUSY");
        released(idle);
        CHECK(idle.start.changes == 0 && idle.reset.changes == 0 && sdk.logs == 0);
        CNetScanDemoDlg active;
        request(active, ENCODER_RESET_BEGIN);
        setBusy(active, flag);
        request(active, ENCODER_RESET_EXECUTE, "BUSY");
        reserved(active);
        request(active, ENCODER_RESET_CANCEL);
        released(active, flag != 2); // A connection check retains control of buttons.
        noSdk();
    }
}
static void not_ready() {
    CNetScanDemoDlg dlg;
    g_pSysInfo = nullptr;
    request(dlg, ENCODER_RESET_BEGIN, "NOT_READY");
    released(dlg);
    CHECK(dlg.start.changes == 0 && dlg.reset.changes == 0);
    g_pSysInfo = &systemInfo;
    request(dlg, ENCODER_RESET_BEGIN);
    g_pSysInfo = nullptr;
    request(dlg, ENCODER_RESET_EXECUTE, "NOT_READY");
    reserved(dlg);
    request(dlg, ENCODER_RESET_CANCEL);
    released(dlg);
    noSdk();
}
enum SafetyGuard { INIT_FAILED, DISCONNECTED, INFO_NULL, J62_ON, OUTPUT_UNKNOWN, AIR_INK_OPEN };
static void setSafetyGuard(CNetScanDemoDlg& dlg, SafetyGuard guard, bool blocked) {
    switch (guard) {
    case INIT_FAILED: dlg.m_bDeviceInitialized = !blocked; break;
    case DISCONNECTED: sdk.connected = !blocked; break;
    case INFO_NULL: sdk.infoAvailable = !blocked; break;
    case J62_ON: sdk.latestInfo.nIoState[0] = blocked ? 0x8 : 0; break;
    case OUTPUT_UNKNOWN: dlg.m_bPressInkOutputUnknown = blocked; break;
    case AIR_INK_OPEN: dlg.m_bAirInkDialogOpen = blocked; break;
    }
}
static void rejected_safety_guard(SafetyGuard guard, const char* expected) {
    // Reject BEGIN, reject RESET then retry, and reject RESET then CANCEL while
    // still unsafe. A failed safety check must neither reset nor poison the cache.
    for (int phase = 0; phase < 3; ++phase) {
        clearEnvironment();
        CNetScanDemoDlg dlg;
        if (phase != 0) request(dlg, ENCODER_RESET_BEGIN);
        setSafetyGuard(dlg, guard, true);
        CHECK(g_pSysInfo != nullptr && g_pSysInfo->nIoState[0] == 0);
        request(dlg, phase == 0 ? ENCODER_RESET_BEGIN : ENCODER_RESET_EXECUTE, expected);
        noSdk();
        if (phase == 0) released(dlg);
        else reserved(dlg);
        CHECK(dlg.start.changes == (phase == 0 ? 0 : 1));
        CHECK(dlg.reset.changes == (phase == 0 ? 0 : 1));
        CHECK(sdk.logs == (phase == 0 ? 0 : 1));
        if (phase == 2) {
            const int connections = sdk.connectionChecks, queries = sdk.infoQueries;
            request(dlg, ENCODER_RESET_CANCEL);
            released(dlg);
            CHECK(sdk.connectionChecks == connections && sdk.infoQueries == queries);
            noSdk();
        }
        setSafetyGuard(dlg, guard, false);
        if (phase != 1) request(dlg, ENCODER_RESET_BEGIN);
        request(dlg, ENCODER_RESET_EXECUTE);
        reserved(dlg, 41, 101, 1);
        CHECK(sdk.resets == 1 && sdk.reads == 1);
        request(dlg, ENCODER_RESET_END);
        released(dlg);
    }
}
static void initialization_failed() { rejected_safety_guard(INIT_FAILED, "NOT_READY"); }
static void device_disconnected() { rejected_safety_guard(DISCONNECTED, "NOT_READY"); }
static void latest_info_null() { rejected_safety_guard(INFO_NULL, "NOT_READY"); }
static void j62_output_on() { rejected_safety_guard(J62_ON, "BUSY"); }
static void output_state_unknown() { rejected_safety_guard(OUTPUT_UNKNOWN, "BUSY"); }
static void air_ink_window_open() { rejected_safety_guard(AIR_INK_OPEN, "BUSY"); }
static void other_io_bits_safe() {
    for (int bits : {0x1, 0x2, 0x4, 0x10, 0x17}) {
        clearEnvironment();
        CNetScanDemoDlg dlg;
        systemInfo.nIoState[0] = 0x8; // Stale cached J62 ON must not be consulted.
        sdk.latestInfo.nIoState[0] = bits;
        request(dlg, ENCODER_RESET_BEGIN);
        CHECK(sdk.connectionChecks == 1 && sdk.infoQueries == 1);
        noSdk();
        request(dlg, ENCODER_RESET_EXECUTE);
        CHECK(sdk.connectionChecks == 2 && sdk.infoQueries == 2);
        reserved(dlg, 41, 101, 1);
        CHECK(sdk.resets == 1 && sdk.reads == 1);
        request(dlg, ENCODER_RESET_END);
        released(dlg);
    }
}
static void reset_success_cached() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    sdk.trace.clear();
    request(dlg, ENCODER_RESET_EXECUTE);
    reserved(dlg, 41, 101, 1);
    CHECK(sdk.resets == 1 && sdk.reads == 1);
    CHECK(sdk.lastPosition == ZERO_POSITION);
    CHECK((sdk.trace == std::vector<std::string>{"reset", "read", "log"}));
    sdk.succeeds = false;
    request(dlg, ENCODER_RESET_EXECUTE);
    CHECK(sdk.resets == 1 && sdk.reads == 1);
    reserved(dlg, 41, 101, 1);
}
static void reset_failure_cached() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    sdk.succeeds = false;
    // Readback at ZERO_POSITION must not turn a failed SDK result into success.
    sdk.readback = ZERO_POSITION;
    request(dlg, ENCODER_RESET_EXECUTE, "FAILED");
    reserved(dlg, 41, 101, -1);
    CHECK(sdk.resets == 1 && sdk.reads == 1 && sdk.lastPosition == ZERO_POSITION);
    sdk.succeeds = true;
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_EXECUTE, "FAILED");
    request(dlg, ENCODER_RESET_END, "NOT_RESET");
    reserved(dlg, 41, 101, -1);
    CHECK(sdk.resets == 1 && sdk.reads == 1);
    request(dlg, ENCODER_RESET_CANCEL);
    request(dlg, ENCODER_RESET_BEGIN, nullptr, 41, 102);
    request(dlg, ENCODER_RESET_EXECUTE, nullptr, 41, 102);
    reserved(dlg, 41, 102, 1);
    CHECK(sdk.resets == 2 && sdk.reads == 2);
}
static void end_without_reset() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_END, "NOT_RESET");
    reserved(dlg);
    CHECK(dlg.start.changes == 1 && dlg.reset.changes == 1);
    noSdk();
}
static void end_releases() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_EXECUTE);
    request(dlg, ENCODER_RESET_END);
    released(dlg);
    CHECK(dlg.start.changes == 2 && dlg.reset.changes == 2);
    request(dlg, ENCODER_RESET_EXECUTE, "NOT_RESERVED");
    request(dlg, ENCODER_RESET_END, "NOT_RESERVED");
    CHECK(sdk.resets == 1 && sdk.reads == 1);
    // A new reservation, even reusing the token, has its own SDK-result cache.
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_EXECUTE);
    CHECK(sdk.resets == 2 && sdk.reads == 2);
}
static void cancel_idempotent_old_token() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_CANCEL);
    CHECK(dlg.start.changes == 0 && dlg.reset.changes == 0);
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_CANCEL);
    released(dlg);
    request(dlg, ENCODER_RESET_CANCEL);
    CHECK(dlg.start.changes == 2 && dlg.reset.changes == 2);
    request(dlg, ENCODER_RESET_BEGIN, nullptr, 41, 102);
    request(dlg, ENCODER_RESET_CANCEL); // Late cancellation of token 101.
    reserved(dlg, 41, 102);
    CHECK(dlg.start.changes == 3 && dlg.reset.changes == 3);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 41, 102);
    released(dlg);
    noSdk();
}
static void disconnect_owner_only() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 42, 0);
    reserved(dlg);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 41, 0);
    released(dlg);
    request(dlg, ENCODER_RESET_BEGIN, nullptr, 42, 102);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 41, 0);
    reserved(dlg, 42, 102);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 42, 0);
    released(dlg);
    noSdk();
}
static void token_zero_cannot_reset_or_end() {
    CNetScanDemoDlg dlg;
    request(dlg, ENCODER_RESET_BEGIN);
    for (int completed = 0; completed < 2; ++completed) {
        for (SOCKET owner : {SOCKET(41), SOCKET(42)}) {
            request(dlg, ENCODER_RESET_EXECUTE, "NOT_RESERVED", owner, 0);
            request(dlg, ENCODER_RESET_END, "NOT_RESERVED", owner, 0);
            reserved(dlg, 41, 101, completed);
        }
        CHECK(dlg.start.changes == 1 && dlg.reset.changes == 1);
        if (!completed) {
            noSdk();
            request(dlg, ENCODER_RESET_EXECUTE);
        }
    }
    CHECK(sdk.resets == 1 && sdk.reads == 1);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 42, 0);
    reserved(dlg, 41, 101, 1);
    request(dlg, ENCODER_RESET_CANCEL, nullptr, 41, 0);
    released(dlg);
    CHECK(sdk.resets == 1 && sdk.reads == 1);
}
static void stopping_guards() {
    for (int closing = 0; closing < 2; ++closing) {
        clearEnvironment();
        CNetScanDemoDlg idle;
        idle.m_bClosing = closing;
        idle.m_lStopSocketServer = !closing;
        request(idle, ENCODER_RESET_BEGIN, "STOPPING");
        released(idle);
        CNetScanDemoDlg active;
        request(active, ENCODER_RESET_BEGIN);
        active.m_bClosing = closing;
        active.m_lStopSocketServer = !closing;
        request(active, ENCODER_RESET_BEGIN, "STOPPING");
        request(active, ENCODER_RESET_EXECUTE, "STOPPING");
        reserved(active);
        request(active, ENCODER_RESET_CANCEL);
        released(active, !closing);
        noSdk();
    }
}
static void null_and_unreserved() {
    CNetScanDemoDlg dlg;
    CHECK(dlg.OnSocketEncoderControl(0, 0) == 0);
    request(dlg, ENCODER_RESET_EXECUTE, "NOT_RESERVED");
    request(dlg, ENCODER_RESET_END, "NOT_RESERVED");
    released(dlg);
    CHECK(dlg.start.changes == 0 && dlg.reset.changes == 0 && sdk.logs == 0);
    noSdk();
}
static void direct_sdk_result() {
    CNetScanDemoDlg dlg;
    CHECK(dlg.ExecuteEncoderReset() == TRUE);
    sdk.succeeds = false;
    sdk.readback = ZERO_POSITION;
    CHECK(dlg.ExecuteEncoderReset() == FALSE);
    CHECK(sdk.resets == 2 && sdk.reads == 2 && sdk.logs == 2);
    CHECK(sdk.lastPosition == ZERO_POSITION);
}
static void command_boundaries() {
    const char* expected[] = {"ENCODER_RESET_BEGIN", "RESET_ENCODER",
                             "ENCODER_RESET_END", "ENCODER_RESET_CANCEL"};
    const int actions[] = {ENCODER_RESET_BEGIN, ENCODER_RESET_EXECUTE,
                           ENCODER_RESET_END, ENCODER_RESET_CANCEL};
    CHECK(_countof(ENCODER_COMMAND_NAMES) == _countof(expected));
    for (int i = 0; i < 4; ++i) {
        CHECK(actions[i] == i);
        CHECK(std::strcmp(ENCODER_COMMAND_NAMES[i], expected[i]) == 0);
        const std::string name = expected[i];
        std::string lower = name;
        for (char& ch : lower) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
        CHECK(GetSocketEncoderAction(name) == i);
        CHECK(GetSocketEncoderAction(lower) == i);
        CHECK(GetSocketEncoderAction(name + " TOKEN=123") == i);
        CHECK(GetSocketEncoderAction(lower + "\tTOKEN=123") == i);
        for (size_t n = 0; n < name.size(); ++n)
            CHECK(GetSocketEncoderAction(name.substr(0, n)) == -1);
        for (const char* suffix : {"X", "_EXTRA", "=", "\r", "\n", ":", "\v"})
            CHECK(GetSocketEncoderAction(name + suffix) == -1);
        CHECK(GetSocketEncoderAction(name + std::string(1, '\0')) == -1);
        CHECK(GetSocketEncoderAction(" " + name) == -1);
    }
    CHECK(GetSocketEncoderAction("HELLO PLC") == -1);
    noSdk();
}

int main(int argc, char** argv) {
    const struct { const char* name; void (*run)(); } cases[] = {
        {"begin_only", begin_only},
        {"begin_idempotent", begin_idempotent},
        {"foreign_reservations", foreign_reservations},
        {"busy_guards", busy_guards},
        {"not_ready", not_ready},
        {"initialization_failed", initialization_failed},
        {"device_disconnected", device_disconnected},
        {"latest_info_null", latest_info_null},
        {"j62_output_on", j62_output_on},
        {"output_state_unknown", output_state_unknown},
        {"air_ink_window_open", air_ink_window_open},
        {"other_io_bits_safe", other_io_bits_safe},
        {"reset_success_cached", reset_success_cached},
        {"reset_failure_cached", reset_failure_cached},
        {"end_without_reset", end_without_reset},
        {"end_releases", end_releases},
        {"cancel_idempotent_old_token", cancel_idempotent_old_token},
        {"disconnect_owner_only", disconnect_owner_only},
        {"token_zero_cannot_reset_or_end", token_zero_cannot_reset_or_end},
        {"stopping_guards", stopping_guards},
        {"null_and_unreserved", null_and_unreserved},
        {"direct_sdk_result", direct_sdk_result},
        {"command_boundaries", command_boundaries},
    };
    try {
        CHECK(argc == 2);
        for (const auto& test : cases) if (std::strcmp(argv[1], test.name) == 0) {
            test.run();
            std::cout << "PASS " << test.name << '\n';
            return 0;
        }
        throw std::runtime_error("Unknown scenario");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
'''


class EncoderResetMfcTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if os.name != "nt":
            raise unittest.SkipTest("The hardware-free C++ harness requires Windows/MSVC")
        # Do not classify an available compiler's compile errors as an unavailable
        # toolchain: source drift/compilation errors must fail this suite.
        developer_environment = bool(os.environ.get("VSCMD_VER") and shutil.which("cl"))
        if not developer_environment and not VSDEVCMD.is_file():
            raise unittest.SkipTest("MSVC x64 developer environment / VS18 Community not installed")
        declarations, methods = _production_fragments()
        cls._temporary = tempfile.TemporaryDirectory(prefix="encoder-reset-mfc-")
        cls.addClassCleanup(cls._temporary.cleanup)
        directory = Path(cls._temporary.name)
        cls.executable = directory / "encoder_reset_test.exe"
        (directory / "encoder_reset_test.cpp").write_text(
            PREAMBLE + declarations + FAKE_DIALOG + methods + SCENARIOS, encoding="utf-8"
        )
        compile_command = (
            'cl /nologo /EHsc /std:c++17 /utf-8 /W4 '
            '/Fe:encoder_reset_test.exe encoder_reset_test.cpp'
        )
        setup = "" if developer_environment else f'call "{VSDEVCMD}" -arch=x64 -host_arch=x64\nif errorlevel 1 exit /b %errorlevel%\n'
        (directory / "build.cmd").write_text(
            "@echo off\n" + setup + compile_command + "\nexit /b %errorlevel%\n",
            encoding="ascii",
        )
        build = subprocess.run(
            [os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", "build.cmd"],
            cwd=directory, capture_output=True, timeout=120,
        )
        if build.returncode:
            raise AssertionError(
                "Console harness compilation failed (no vendor libraries are linked):\n"
                + (build.stdout + build.stderr).decode(errors="replace")
            )

    def run_scenario(self, name):
        result = subprocess.run(
            [str(self.executable), name], cwd=self.executable.parent,
            capture_output=True, timeout=15,
        )
        self.assertEqual(result.returncode, 0, (result.stdout + result.stderr).decode(errors="replace"))
        self.assertEqual(result.stdout.decode().strip(), f"PASS {name}")

    def test_begin_only(self):
        self.run_scenario("begin_only")

    def test_begin_idempotent(self):
        self.run_scenario("begin_idempotent")

    def test_foreign_owner_and_token(self):
        self.run_scenario("foreign_reservations")

    def test_busy_print_check_and_press_ink(self):
        self.run_scenario("busy_guards")

    def test_system_info_not_ready(self):
        self.run_scenario("not_ready")

    def test_failed_initialization_is_not_ready(self):
        self.run_scenario("initialization_failed")

    def test_disconnected_device_is_not_ready(self):
        self.run_scenario("device_disconnected")

    def test_latest_device_info_null_is_not_ready(self):
        self.run_scenario("latest_info_null")

    def test_latest_j62_output_on_is_busy(self):
        self.run_scenario("j62_output_on")

    def test_unknown_press_ink_output_is_busy(self):
        self.run_scenario("output_state_unknown")

    def test_air_ink_dialog_open_is_busy(self):
        self.run_scenario("air_ink_window_open")

    def test_other_io_bits_allowed_and_latest_info_used(self):
        self.run_scenario("other_io_bits_safe")

    def test_token_zero_cannot_reset_or_end(self):
        self.run_scenario("token_zero_cannot_reset_or_end")

    def test_reset_success_is_cached(self):
        self.run_scenario("reset_success_cached")

    def test_reset_failure_is_cached(self):
        self.run_scenario("reset_failure_cached")

    def test_end_without_reset_fails(self):
        self.run_scenario("end_without_reset")

    def test_end_releases_and_restores_buttons(self):
        self.run_scenario("end_releases")

    def test_cancel_idempotent_and_old_token_isolated(self):
        self.run_scenario("cancel_idempotent_old_token")

    def test_disconnect_token_zero_only_releases_owner(self):
        self.run_scenario("disconnect_owner_only")

    def test_closing_and_stop_reject_but_cancel_releases(self):
        self.run_scenario("stopping_guards")

    def test_null_and_unreserved_requests(self):
        self.run_scenario("null_and_unreserved")

    def test_execute_encoder_reset_returns_sdk_result(self):
        self.run_scenario("direct_sdk_result")

    def test_command_name_classification_boundaries(self):
        self.run_scenario("command_boundaries")


if __name__ == "__main__":
    unittest.main(verbosity=2)
