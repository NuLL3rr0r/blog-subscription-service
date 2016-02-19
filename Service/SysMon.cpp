/**
 * @file
 * @author  Mohammad S. Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 Mohammad S. Babaei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * System monitor utility that watches our system resources such as CPU, memory,
 * ... and shows them in a nice graphical way.
 */


#include <list>
#include <map>
#include <cmath>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <Wt/Chart/WCartesianChart>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTable>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTimer>
#include <Wt/WWidget>
#include <statgrab.h>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Utility.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "Div.hpp"
#include "SysMon.hpp"

#define     MAX_INSTANTS         60

using namespace std;
using namespace boost;
using namespace Wt;
using namespace Wt::Chart;
using namespace CoreLib;
using namespace Service;

struct SysMon::Impl : public Wt::WObject
{
public:
    enum class Cpu : unsigned char {
        User,
        Kernel,
        Idle,
        IoWait,
        Swap,
        Nice
    };

    enum class Memory : unsigned char {
        Total,
        Free,
        Used,
        Cache
    };

    enum class Swap : unsigned char {
        Total,
        Used,
        Free
    };

    enum class VirtualMemory : unsigned char {
        Total,
        Used,
        Free
    };

    typedef std::map<Cpu, double> CpuInstant;
    typedef std::map<Memory, double> MemoryInstant;
    typedef std::map<Swap, double> SwapInstant;
    typedef std::map<VirtualMemory, double> VirtualMemoryInstant;

    typedef std::unordered_map<sg_host_state, Wt::WString,
        CoreLib::Utility::Hasher<sg_host_state>> HostStateHashTable;

public:
    Wt::WTimer *Timer;

    HostStateHashTable HostState;

    std::list<CpuInstant> CpuUsageCache;
    Wt::WStandardItemModel *CpuUsageModel;

    std::list<MemoryInstant> MemoryUsageCache;
    Wt::WStandardItemModel *MemoryUsageModel;

    std::list<SwapInstant> SwapUsageCache;
    Wt::WStandardItemModel *SwapUsageModel;

    std::list<VirtualMemoryInstant> VirtualMemoryUsageCache;
    Wt::WStandardItemModel *VirtualMemoryUsageModel;

    Wt::WContainerWidget *HostInfoHorizontalDiv;
    Wt::WContainerWidget *HostInfoVerticalDiv;
    Wt::WContainerWidget *DiskInfoDiv;
    Wt::WContainerWidget *NetworkInfoDiv;

public:
    Impl();
    ~Impl();

private:
    void RefreshResourceUsage();

public:
    void Initialize();
};

SysMon::SysMon()
    : Page(),
    m_pimpl(make_unique<SysMon::Impl>())
{
    this->clear();
    this->setId("SysMonPage");
    this->addWidget(this->Layout());

    m_pimpl->Initialize();
}

SysMon::~SysMon() = default;

void SysMon::Pause()
{
    if (m_pimpl->Timer == nullptr)
        return;

    m_pimpl->Timer->stop();
}

void SysMon::Resume()
{
    if (m_pimpl->Timer == nullptr)
        return;

    m_pimpl->Timer->start();
}

WWidget *SysMon::Layout()
{
    Div *container = new Div("SysMon", "container-fluid");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/system-monitor-fa.wtml";
    } else {
        file = "../templates/system-monitor.wtml";
    }

    /// Read the template, otherwise return
    if (!CoreLib::FileSystem::Read(file, htmlData)) {
        return container;
    }


    /// Host Info
    m_pimpl->HostInfoHorizontalDiv = new Div(container, "HostInfoHorizontalDiv");
    m_pimpl->HostInfoVerticalDiv = new Div(container, "HostInfoVerticalDiv");


    /// CPU Model
    m_pimpl->CpuUsageModel = new WStandardItemModel(60, 7, this);
    m_pimpl->CpuUsageModel->setHeaderData(0, tr("system-monitor-cpu-percents-user"));
    m_pimpl->CpuUsageModel->setHeaderData(1, tr("system-monitor-cpu-percents-kernel"));
    m_pimpl->CpuUsageModel->setHeaderData(2, tr("system-monitor-cpu-percents-idle"));
    m_pimpl->CpuUsageModel->setHeaderData(3, tr("system-monitor-cpu-percents-iowait"));
    m_pimpl->CpuUsageModel->setHeaderData(4, tr("system-monitor-cpu-percents-swap"));
    m_pimpl->CpuUsageModel->setHeaderData(5, tr("system-monitor-cpu-percents-nice"));
    m_pimpl->CpuUsageModel->setHeaderData(6, WString("-"));


    /// Memory Model
    m_pimpl->MemoryUsageModel = new WStandardItemModel(60, 5, this);
    m_pimpl->MemoryUsageModel->setHeaderData(0, tr("system-monitor-memory-stats-total"));
    m_pimpl->MemoryUsageModel->setHeaderData(1, tr("system-monitor-memory-stats-free"));
    m_pimpl->MemoryUsageModel->setHeaderData(2, tr("system-monitor-memory-stats-used"));
    m_pimpl->MemoryUsageModel->setHeaderData(3, tr("system-monitor-memory-stats-cache"));
    m_pimpl->MemoryUsageModel->setHeaderData(4, WString("-"));


    /// Swap Model
    m_pimpl->SwapUsageModel = new WStandardItemModel(60, 4, this);
    m_pimpl->SwapUsageModel->setHeaderData(0, tr("system-monitor-swap-stats-total"));
    m_pimpl->SwapUsageModel->setHeaderData(1, tr("system-monitor-swap-stats-used"));
    m_pimpl->SwapUsageModel->setHeaderData(2, tr("system-monitor-swap-stats-free"));
    m_pimpl->SwapUsageModel->setHeaderData(3, WString("-"));


    /// Virtual Memory Model
    m_pimpl->VirtualMemoryUsageModel = new WStandardItemModel(60, 4, this);
    m_pimpl->VirtualMemoryUsageModel->setHeaderData(0, tr("system-monitor-virtual-memory-stats-total"));
    m_pimpl->VirtualMemoryUsageModel->setHeaderData(1, tr("system-monitor-virtual-memory-stats-used"));
    m_pimpl->VirtualMemoryUsageModel->setHeaderData(2, tr("system-monitor-virtual-memory-stats-free"));
    m_pimpl->VirtualMemoryUsageModel->setHeaderData(3, WString("-"));


    /// CPU Graph
    WCartesianChart *cpuUsageChart = new WCartesianChart();
    cpuUsageChart->setBackground(gray);
    cpuUsageChart->setModel(m_pimpl->CpuUsageModel);
    cpuUsageChart->setXSeriesColumn(m_pimpl->CpuUsageModel->columnCount() - 1);
    cpuUsageChart->setLegendEnabled(true);
    cpuUsageChart->setLegendLocation(LegendOutside, Right, AlignMiddle);

    cpuUsageChart->setType(ScatterPlot);
    cpuUsageChart->axis(XAxis).setScale(LinearScale);
    cpuUsageChart->axis(YAxis).setScale(LinearScale);

    cpuUsageChart->setPlotAreaPadding(120, Right);
    cpuUsageChart->setPlotAreaPadding(30, Top | Bottom);

    for (int i = 0; i < m_pimpl->CpuUsageModel->columnCount() - 1; ++i) {
        WDataSeries s(i, CurveSeries);
        s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        cpuUsageChart->addSeries(s);
    }

    cpuUsageChart->resize(600, 330);

    cpuUsageChart->setMargin(10, Top | Bottom);
    cpuUsageChart->setMargin(WLength::Auto, Left | Right);


    /// Memory Graph
    WCartesianChart *memoryUsageChart = new WCartesianChart();
    memoryUsageChart->setBackground(gray);
    memoryUsageChart->setModel(m_pimpl->MemoryUsageModel);
    memoryUsageChart->setXSeriesColumn(m_pimpl->MemoryUsageModel->columnCount() - 1);
    memoryUsageChart->setLegendEnabled(true);
    memoryUsageChart->setLegendLocation(LegendOutside, Right, AlignMiddle);

    memoryUsageChart->setType(ScatterPlot);
    memoryUsageChart->axis(XAxis).setScale(LinearScale);
    memoryUsageChart->axis(YAxis).setScale(LinearScale);

    memoryUsageChart->setPlotAreaPadding(220, Right);
    memoryUsageChart->setPlotAreaPadding(30, Top | Bottom);

    for (int i = 0; i < m_pimpl->MemoryUsageModel->columnCount() - 1; ++i) {
        WDataSeries s(i, CurveSeries);
        s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        memoryUsageChart->addSeries(s);
    }

    memoryUsageChart->resize(600, 330);

    memoryUsageChart->setMargin(10, Top | Bottom);
    memoryUsageChart->setMargin(WLength::Auto, Left | Right);


    /// Swap Graph
    WCartesianChart *swapUsageChart = new WCartesianChart();
    swapUsageChart->setBackground(gray);
    swapUsageChart->setModel(m_pimpl->SwapUsageModel);
    swapUsageChart->setXSeriesColumn(m_pimpl->SwapUsageModel->columnCount() - 1);
    swapUsageChart->setLegendEnabled(true);
    swapUsageChart->setLegendLocation(LegendOutside, Right, AlignMiddle);

    swapUsageChart->setType(ScatterPlot);
    swapUsageChart->axis(XAxis).setScale(LinearScale);
    swapUsageChart->axis(YAxis).setScale(LinearScale);

    swapUsageChart->setPlotAreaPadding(220, Right);
    swapUsageChart->setPlotAreaPadding(30, Top | Bottom);

    for (int i = 0; i < m_pimpl->SwapUsageModel->columnCount() - 1; ++i) {
        WDataSeries s(i, CurveSeries);
        s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        swapUsageChart->addSeries(s);
    }

    swapUsageChart->resize(600, 330);

    swapUsageChart->setMargin(10, Top | Bottom);
    swapUsageChart->setMargin(WLength::Auto, Left | Right);


    /// Virtual Memory Graph
    WCartesianChart *virtualMemoryUsageChart = new WCartesianChart();
    virtualMemoryUsageChart->setBackground(gray);
    virtualMemoryUsageChart->setModel(m_pimpl->VirtualMemoryUsageModel);
    virtualMemoryUsageChart->setXSeriesColumn(m_pimpl->VirtualMemoryUsageModel->columnCount() - 1);
    virtualMemoryUsageChart->setLegendEnabled(true);
    virtualMemoryUsageChart->setLegendLocation(LegendOutside, Right, AlignMiddle);

    virtualMemoryUsageChart->setType(ScatterPlot);
    virtualMemoryUsageChart->axis(XAxis).setScale(LinearScale);
    virtualMemoryUsageChart->axis(YAxis).setScale(LinearScale);

    virtualMemoryUsageChart->setPlotAreaPadding(220, Right);
    virtualMemoryUsageChart->setPlotAreaPadding(30, Top | Bottom);

    for (int i = 0; i < m_pimpl->VirtualMemoryUsageModel->columnCount() - 1; ++i) {
        WDataSeries s(i, CurveSeries);
        s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        virtualMemoryUsageChart->addSeries(s);
    }

    virtualMemoryUsageChart->resize(600, 330);

    virtualMemoryUsageChart->setMargin(10, Top | Bottom);
    virtualMemoryUsageChart->setMargin(WLength::Auto, Left | Right);


    /// Disk Info
    m_pimpl->DiskInfoDiv = new Div(container, "DiskInfoDiv");


    /// Net Info
    m_pimpl->NetworkInfoDiv = new Div(container, "NetworkInfoDiv");


    /// Fill the template
    WTemplate *tmpl = new WTemplate(container);
    tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

    tmpl->bindWidget("host-info-horizontal", m_pimpl->HostInfoHorizontalDiv);
    tmpl->bindWidget("host-info-vertical", m_pimpl->HostInfoVerticalDiv);
    tmpl->bindWidget("cpu-info", cpuUsageChart);
    tmpl->bindWidget("memory-info", memoryUsageChart);
    tmpl->bindWidget("swap-info", swapUsageChart);
    tmpl->bindWidget("virtual-memory-info", virtualMemoryUsageChart);
    tmpl->bindWidget("disk-info", m_pimpl->DiskInfoDiv);
    tmpl->bindWidget("network-info", m_pimpl->NetworkInfoDiv);

    tmpl->bindWidget("host-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-host-info"))));
    tmpl->bindWidget("cpu-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-cpu-percents"))));
    tmpl->bindWidget("memory-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-memory-stats"))));
    tmpl->bindWidget("swap-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-swap-stats"))));
    tmpl->bindWidget("virtual-memory-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-virtual-memory-stats"))));
    tmpl->bindWidget("disk-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-disk-io-stats"))));
    tmpl->bindWidget("network-info-title",
                     new WText(WString("<h4>{1}</h4>").arg(tr("system-monitor-network-io-stats"))));

    return container;
}

SysMon::Impl::Impl()
    : Timer(nullptr),
    HostState {
        {sg_unknown_configuration, tr("system-monitor-host-info-host-state-unknown-configuration")},
        {sg_physical_host, tr("system-monitor-host-info-host-state-physical-host")},
        {sg_virtual_machine, tr("system-monitor-host-info-host-state-virtual-machine")},
        {sg_paravirtual_machine, tr("system-monitor-host-info-host-state-paravirtual-machine")},
        {sg_hardware_virtualized, tr("system-monitor-host-info-host-state-hardware-virtualized")}
    }
{

}

SysMon::Impl::~Impl() = default;

void SysMon::Impl::RefreshResourceUsage()
{
    sg_cpu_percents *cpuPercents;
    sg_mem_stats *memStats;
    sg_swap_stats *swapStats;
    sg_host_info *hostInfo;
    sg_disk_io_stats *diskIoStats;
    sg_network_io_stats *networkIoStats;

    size_t cpuPercentsEntries = 0;
    size_t memStatsEntries = 0;
    size_t swapStatsEntries = 0;
    size_t hostInfoEntries = 0;
    size_t diskIoStatsEntries = 0;
    size_t networkIoStatsEntries = 0;


    /// Get the host info
    if ((hostInfo = sg_get_host_info(&hostInfoEntries)) != NULL) {
        HostInfoHorizontalDiv->clear();
        HostInfoVerticalDiv->clear();

        WTable *horizontalHostInfoTable = new WTable(HostInfoHorizontalDiv);
        horizontalHostInfoTable->setStyleClass("table table-hover");
        horizontalHostInfoTable->setHeaderCount(1, Orientation::Horizontal);

        WTable *verticalHostInfoTable = new WTable(HostInfoVerticalDiv);
        verticalHostInfoTable->setStyleClass("table table-hover");
        verticalHostInfoTable->setHeaderCount(1, Orientation::Vertical);

        horizontalHostInfoTable->elementAt(0, 0)->addWidget(new WText(tr("system-monitor-host-info-os-name")));
        horizontalHostInfoTable->elementAt(0, 1)->addWidget(new WText(tr("system-monitor-host-info-os-release")));
        horizontalHostInfoTable->elementAt(0, 2)->addWidget(new WText(tr("system-monitor-host-info-os-version")));
        horizontalHostInfoTable->elementAt(0, 3)->addWidget(new WText(tr("system-monitor-host-info-platform")));
        horizontalHostInfoTable->elementAt(0, 4)->addWidget(new WText(tr("system-monitor-host-info-hostname")));
        horizontalHostInfoTable->elementAt(0, 5)->addWidget(new WText(tr("system-monitor-host-info-bitwidth")));
        horizontalHostInfoTable->elementAt(0, 6)->addWidget(new WText(tr("system-monitor-host-info-host-state")));
        horizontalHostInfoTable->elementAt(0, 7)->addWidget(new WText(tr("system-monitor-host-info-ncpus")));
        horizontalHostInfoTable->elementAt(0, 8)->addWidget(new WText(tr("system-monitor-host-info-maxcpus")));
        horizontalHostInfoTable->elementAt(0, 9)->addWidget(new WText(tr("system-monitor-host-info-uptime")));
        horizontalHostInfoTable->elementAt(0, 10)->addWidget(new WText(tr("system-monitor-host-info-systime")));

        verticalHostInfoTable->elementAt(0, 0)->addWidget(new WText(tr("system-monitor-host-info-os-name")));
        verticalHostInfoTable->elementAt(1, 0)->addWidget(new WText(tr("system-monitor-host-info-os-release")));
        verticalHostInfoTable->elementAt(2, 0)->addWidget(new WText(tr("system-monitor-host-info-os-version")));
        verticalHostInfoTable->elementAt(3, 0)->addWidget(new WText(tr("system-monitor-host-info-platform")));
        verticalHostInfoTable->elementAt(4, 0)->addWidget(new WText(tr("system-monitor-host-info-hostname")));
        verticalHostInfoTable->elementAt(5, 0)->addWidget(new WText(tr("system-monitor-host-info-bitwidth")));
        verticalHostInfoTable->elementAt(6, 0)->addWidget(new WText(tr("system-monitor-host-info-host-state")));
        verticalHostInfoTable->elementAt(7, 0)->addWidget(new WText(tr("system-monitor-host-info-ncpus")));
        verticalHostInfoTable->elementAt(8, 0)->addWidget(new WText(tr("system-monitor-host-info-maxcpus")));
        verticalHostInfoTable->elementAt(9, 0)->addWidget(new WText(tr("system-monitor-host-info-uptime")));
        verticalHostInfoTable->elementAt(10, 0)->addWidget(new WText(tr("system-monitor-host-info-systime")));

        horizontalHostInfoTable->elementAt(1, 0)->addWidget(new WText(WString(hostInfo->os_name)));
        horizontalHostInfoTable->elementAt(1, 1)->addWidget(new WText(WString(hostInfo->os_release)));
        horizontalHostInfoTable->elementAt(1, 2)->addWidget(new WText(WString(hostInfo->os_version)));
        horizontalHostInfoTable->elementAt(1, 3)->addWidget(new WText(WString(hostInfo->platform)));
        horizontalHostInfoTable->elementAt(1, 4)->addWidget(new WText(WString(hostInfo->hostname)));
        horizontalHostInfoTable->elementAt(1, 5)->addWidget(new WText(lexical_cast<string>(hostInfo->bitwidth)));
        horizontalHostInfoTable->elementAt(1, 6)->addWidget(new WText(lexical_cast<string>(HostState[hostInfo->host_state])));
        horizontalHostInfoTable->elementAt(1, 7)->addWidget(new WText(lexical_cast<string>(hostInfo->ncpus)));
        horizontalHostInfoTable->elementAt(1, 8)->addWidget(new WText(lexical_cast<string>(hostInfo->maxcpus)));
        horizontalHostInfoTable->elementAt(1, 9)->addWidget(new WText(lexical_cast<string>(hostInfo->uptime)));
        horizontalHostInfoTable->elementAt(1, 10)->addWidget(new WText(lexical_cast<string>(hostInfo->systime)));

        verticalHostInfoTable->elementAt(0, 1)->addWidget(new WText(WString(hostInfo->os_name)));
        verticalHostInfoTable->elementAt(1, 1)->addWidget(new WText(WString(hostInfo->os_release)));
        verticalHostInfoTable->elementAt(2, 1)->addWidget(new WText(WString(hostInfo->os_version)));
        verticalHostInfoTable->elementAt(3, 1)->addWidget(new WText(WString(hostInfo->platform)));
        verticalHostInfoTable->elementAt(4, 1)->addWidget(new WText(WString(hostInfo->hostname)));
        verticalHostInfoTable->elementAt(5, 1)->addWidget(new WText(lexical_cast<string>(hostInfo->bitwidth)));
        verticalHostInfoTable->elementAt(6, 1)->addWidget(new WText(lexical_cast<string>(HostState[hostInfo->host_state])));
        verticalHostInfoTable->elementAt(7, 1)->addWidget(new WText(lexical_cast<string>(hostInfo->ncpus)));
        verticalHostInfoTable->elementAt(8, 1)->addWidget(new WText(lexical_cast<string>(hostInfo->maxcpus)));
        verticalHostInfoTable->elementAt(9, 1)->addWidget(new WText(lexical_cast<string>(hostInfo->uptime)));
        verticalHostInfoTable->elementAt(10, 1)->addWidget(new WText(lexical_cast<string>(hostInfo->systime)));
    }

    /// Shift and fill the CPU usage cache and model
    if ((cpuPercents = sg_get_cpu_percents(&cpuPercentsEntries)) != NULL) {
        CpuInstant cpuInstant;
        cpuInstant[Cpu::User] = cpuPercents->user;
        cpuInstant[Cpu::Kernel] = cpuPercents->kernel;
        cpuInstant[Cpu::Idle] = cpuPercents->idle;
        cpuInstant[Cpu::IoWait] = cpuPercents->iowait;
        cpuInstant[Cpu::Swap] = cpuPercents->swap;
        cpuInstant[Cpu::Nice] = cpuPercents->nice;

        CpuUsageCache.push_back(cpuInstant);
        CpuUsageCache.pop_front();

        int i = 0;
        for (list<CpuInstant>::const_iterator it =
             CpuUsageCache.begin(); it != CpuUsageCache.end(); ++it) {
            cpuInstant = *it;
            CpuUsageModel->setData(i, 0, cpuInstant[Cpu::User]);
            CpuUsageModel->setData(i, 1, cpuInstant[Cpu::Kernel]);
            CpuUsageModel->setData(i, 2, cpuInstant[Cpu::Idle]);
            CpuUsageModel->setData(i, 3, cpuInstant[Cpu::IoWait]);
            CpuUsageModel->setData(i, 4, cpuInstant[Cpu::Swap]);
            CpuUsageModel->setData(i, 5, cpuInstant[Cpu::Nice]);
            CpuUsageModel->setData(i, 6, i);
            ++i;
        }
    }


    /// Shift and fill the memory usage cache and model
    if(((memStats = sg_get_mem_stats(&memStatsEntries)) != NULL)) {
        MemoryInstant memoryInstant;
        memoryInstant[Memory::Total] = 100.0;
        memoryInstant[Memory::Free] = 100.0 * memStats->free / memStats->total;
        memoryInstant[Memory::Used] = 100.0 * memStats->used / memStats->total;
        memoryInstant[Memory::Cache] = 100.0 * memStats->cache / memStats->total;

        MemoryUsageCache.push_back(memoryInstant);
        MemoryUsageCache.pop_front();

        MemoryUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(Utility::CalculateSize(memStats->total))
                    .arg(tr("system-monitor-memory-stats-total")));
        MemoryUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(Utility::CalculateSize(memStats->free))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Free]).str())
                .arg(tr("system-monitor-memory-stats-free"))
                );
        MemoryUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((Utility::CalculateSize(memStats->used)))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Used]).str())
                .arg(tr("system-monitor-memory-stats-used"))
                );
        MemoryUsageModel->setHeaderData(
                    3, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(Utility::CalculateSize(memStats->cache))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Cache]).str())
                .arg(tr("system-monitor-memory-stats-cache"))
                );

        int i = 0;
        for (list<MemoryInstant>::const_iterator it =
             MemoryUsageCache.begin(); it != MemoryUsageCache.end(); ++it) {
            memoryInstant = *it;
            MemoryUsageModel->setData(i, 0, memoryInstant[Memory::Total]);
            MemoryUsageModel->setData(i, 1, memoryInstant[Memory::Free]);
            MemoryUsageModel->setData(i, 2, memoryInstant[Memory::Used]);
            MemoryUsageModel->setData(i, 3, memoryInstant[Memory::Cache]);
            MemoryUsageModel->setData(i, 4, i);
            ++i;
        }
    }


    /// Shift and fill the swap usage cache and model
    if(((swapStats = sg_get_swap_stats(&swapStatsEntries)) != NULL)) {
        SwapInstant swapInstant;
        swapInstant[Swap::Total] = 100.0;
        swapInstant[Swap::Used] = 100.0 * swapStats->used / swapStats->total;
        swapInstant[Swap::Free] = 100.0 * swapStats->free / swapStats->total;

        SwapUsageCache.push_back(swapInstant);
        SwapUsageCache.pop_front();

        SwapUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(Utility::CalculateSize(swapStats->total))
                    .arg(tr("system-monitor-swap-stats-total")));
        SwapUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((Utility::CalculateSize(swapStats->used)))
                    .arg((wformat(L"%.1f") % (!isnan(swapInstant[Swap::Used]) ? swapInstant[Swap::Used] : 0.0)).str())
                .arg(tr("system-monitor-swap-stats-used"))
                );
        SwapUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(Utility::CalculateSize(swapStats->free))
                    .arg((wformat(L"%.1f") % (!isnan(swapInstant[Swap::Free]) ? swapInstant[Swap::Free] : 0.0)).str())
                .arg(tr("system-monitor-swap-stats-free"))
                );

        int i = 0;
        for (list<SwapInstant>::const_iterator it =
             SwapUsageCache.begin(); it != SwapUsageCache.end(); ++it) {
            swapInstant = *it;
            SwapUsageModel->setData(i, 0, swapInstant[Swap::Total]);
            SwapUsageModel->setData(i, 1, swapInstant[Swap::Used]);
            SwapUsageModel->setData(i, 2, swapInstant[Swap::Free]);
            SwapUsageModel->setData(i, 3, i);
            ++i;
        }
    }


    /// Shift and fill the virtual memory usage cache and model
    if (memStatsEntries > 0 && swapStatsEntries > 0) {
        unsigned long long total;
        unsigned long long used;
        unsigned long long free;

        if (!isnan(100.0 * swapStats->total / swapStats->total))
            total = memStats->total + swapStats->total;
        else
            total = memStats->total;

        if (!isnan(100.0 * swapStats->used / swapStats->total))
            used = memStats->used + swapStats->used;
        else
            used = memStats->used;

        if (!isnan(100.0 * swapStats->free / swapStats->total))
            free = memStats->free + swapStats->free;
        else
            free = memStats->free;


        VirtualMemoryInstant virtualMemoryInstant;
        virtualMemoryInstant[VirtualMemory::Total] = 100.0;
        virtualMemoryInstant[VirtualMemory::Used] = 100.0 * used / total;
        virtualMemoryInstant[VirtualMemory::Free] = 100.0 * free / total;

        VirtualMemoryUsageCache.push_back(virtualMemoryInstant);
        VirtualMemoryUsageCache.pop_front();

        VirtualMemoryUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(Utility::CalculateSize(total))
                    .arg(tr("system-monitor-virtual-memory-stats-total")));
        VirtualMemoryUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((Utility::CalculateSize(used)))
                    .arg((wformat(L"%.1f") % virtualMemoryInstant[VirtualMemory::Used]).str())
                .arg(tr("system-monitor-virtual-memory-stats-used"))
                );
        VirtualMemoryUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(Utility::CalculateSize(free))
                    .arg((wformat(L"%.1f") % virtualMemoryInstant[VirtualMemory::Free]).str())
                .arg(tr("system-monitor-virtual-memory-stats-free"))
                );

        int i = 0;
        for (list<VirtualMemoryInstant>::const_iterator it =
             VirtualMemoryUsageCache.begin(); it != VirtualMemoryUsageCache.end(); ++it) {
            virtualMemoryInstant = *it;
            VirtualMemoryUsageModel->setData(i, 0, virtualMemoryInstant[VirtualMemory::Total]);
            VirtualMemoryUsageModel->setData(i, 1, virtualMemoryInstant[VirtualMemory::Used]);
            VirtualMemoryUsageModel->setData(i, 2, virtualMemoryInstant[VirtualMemory::Free]);
            VirtualMemoryUsageModel->setData(i, 3, i);
            ++i;
        }
    }


    /// Get the disk info
    DiskInfoDiv->clear();

    size_t diskTotalRead = 0;
    size_t diskTotalWrite = 0;

    WTable *diskTable = new WTable(DiskInfoDiv);
    diskTable->setStyleClass("table table-hover");
    diskTable->setHeaderCount(1, Orientation::Horizontal);

    diskTable->elementAt(0, 0)->addWidget(new WText(tr("system-monitor-disk-io-stats-disk-name")));
    diskTable->elementAt(0, 1)->addWidget(new WText(tr("system-monitor-disk-io-stats-read-bytes")));
    diskTable->elementAt(0, 2)->addWidget(new WText(tr("system-monitor-disk-io-stats-write-bytes")));
    diskTable->elementAt(0, 3)->addWidget(new WText(tr("system-monitor-disk-io-stats-systime")));

    if ((diskIoStats = sg_get_disk_io_stats_diff(&diskIoStatsEntries)) != NULL) {
        for (size_t i = 0; i < diskIoStatsEntries; ++i) {
            diskTable->elementAt((int)i + 1, 0)->addWidget(new WText(lexical_cast<string>(diskIoStats->disk_name)));
            diskTable->elementAt((int)i + 1, 1)->addWidget(new WText(WString(L"{1}")
                                                                     .arg(Utility::CalculateSize(diskIoStats->read_bytes))));
            diskTable->elementAt((int)i + 1, 2)->addWidget(new WText(WString(L"{1}")
                                                                     .arg(Utility::CalculateSize(diskIoStats->write_bytes))));
            diskTable->elementAt((int)i + 1, 3)->addWidget(new WText(lexical_cast<string>((long)diskIoStats->systime)));

            diskTotalRead += diskIoStats->read_bytes;
            diskTotalWrite += diskIoStats->write_bytes;

            ++diskIoStats;
        }

        diskTable->elementAt((int)diskIoStatsEntries + 1, 0)->addWidget(new WText(tr("system-monitor-disk-io-stats-total")));
        diskTable->elementAt((int)diskIoStatsEntries + 1, 1)->addWidget(new WText(WString(L"{1}")
                                                                                  .arg(Utility::CalculateSize(diskTotalRead))));
        diskTable->elementAt((int)diskIoStatsEntries + 1, 2)->addWidget(new WText(WString(L"{1}")
                                                                                  .arg(Utility::CalculateSize(diskTotalWrite))));
        diskTable->elementAt((int)diskIoStatsEntries + 1, 3)->addWidget(new WText("-"));
    }

    /// Get the netowrk info
    NetworkInfoDiv->clear();

    size_t networkTotalRx = 0;
    size_t networkTotalTx = 0;
    size_t networkTotalPacketsIn = 0;
    size_t networkTotalPacketsOut = 0;
    size_t networkTotalErrorsIn = 0;
    size_t networkTotalErrorsOut = 0;
    size_t networkTotalCollisions = 0;

    WTable *networkTable = new WTable(NetworkInfoDiv);
    networkTable->setStyleClass("table table-hover");
    networkTable->setHeaderCount(1, Orientation::Horizontal);

    networkTable->elementAt(0, 0)->addWidget(new WText(tr("system-monitor-network-io-stats-interface_name")));
    networkTable->elementAt(0, 1)->addWidget(new WText(tr("system-monitor-network-io-stats-tx")));
    networkTable->elementAt(0, 2)->addWidget(new WText(tr("system-monitor-network-io-stats-rx")));
    networkTable->elementAt(0, 3)->addWidget(new WText(tr("system-monitor-network-io-stats-ipackets")));
    networkTable->elementAt(0, 4)->addWidget(new WText(tr("system-monitor-network-io-stats-opackets")));
    networkTable->elementAt(0, 5)->addWidget(new WText(tr("system-monitor-network-io-stats-ierrors")));
    networkTable->elementAt(0, 6)->addWidget(new WText(tr("system-monitor-network-io-stats-oerrors")));
    networkTable->elementAt(0, 7)->addWidget(new WText(tr("system-monitor-network-io-stats-collisions")));
    networkTable->elementAt(0, 8)->addWidget(new WText(tr("system-monitor-network-io-stats-systime")));

    if ((networkIoStats = sg_get_network_io_stats_diff(&networkIoStatsEntries)) != NULL) {
        for (size_t i = 0; i < networkIoStatsEntries; ++i) {
            networkTable->elementAt((int)i + 1, 0)->addWidget(new WText(lexical_cast<string>(networkIoStats->interface_name)));
            networkTable->elementAt((int)i + 1, 1)->addWidget(new WText(WString(L"{1}")
                                                                        .arg(Utility::CalculateSize(networkIoStats->tx))));
            networkTable->elementAt((int)i + 1, 2)->addWidget(new WText(WString(L"{1}")
                                                                        .arg(Utility::CalculateSize(networkIoStats->rx))));
            networkTable->elementAt((int)i + 1, 3)->addWidget(new WText(lexical_cast<string>(networkIoStats->ipackets)));
            networkTable->elementAt((int)i + 1, 4)->addWidget(new WText(lexical_cast<string>(networkIoStats->opackets)));
            networkTable->elementAt((int)i + 1, 5)->addWidget(new WText(lexical_cast<string>(networkIoStats->ierrors)));
            networkTable->elementAt((int)i + 1, 6)->addWidget(new WText(lexical_cast<string>(networkIoStats->oerrors)));
            networkTable->elementAt((int)i + 1, 7)->addWidget(new WText(lexical_cast<string>(networkIoStats->collisions)));
            networkTable->elementAt((int)i + 1, 8)->addWidget(new WText(lexical_cast<string>((long)networkIoStats->systime)));

            networkTotalTx += networkIoStats->tx;
            networkTotalRx += networkIoStats->rx;
            networkTotalPacketsIn += networkIoStats->ipackets;
            networkTotalPacketsOut += networkIoStats->opackets;
            networkTotalErrorsIn += networkIoStats->ierrors;
            networkTotalErrorsOut += networkIoStats->oerrors;
            networkTotalCollisions += networkIoStats->collisions;

            new WBreak(NetworkInfoDiv);

            ++networkIoStats;
        }

        networkTable->elementAt((int)networkIoStatsEntries + 1, 0)->addWidget(new WText(tr("system-monitor-network-io-stats-total")));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 1)->addWidget(new WText(WString(L"{1}")
                                                                                        .arg(Utility::CalculateSize(networkTotalTx))));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 2)->addWidget(new WText(WString(L"{1}")
                                                                                        .arg(Utility::CalculateSize(networkTotalRx))));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 3)->addWidget(new WText(lexical_cast<string>(networkTotalPacketsIn)));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 4)->addWidget(new WText(lexical_cast<string>(networkTotalPacketsOut)));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 5)->addWidget(new WText(lexical_cast<string>(networkTotalErrorsIn)));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 6)->addWidget(new WText(lexical_cast<string>(networkTotalErrorsOut)));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 7)->addWidget(new WText(lexical_cast<string>(networkTotalCollisions)));
        networkTable->elementAt((int)networkIoStatsEntries + 1, 8)->addWidget(new WText("-"));

        new WBreak(NetworkInfoDiv);
    }
}

void SysMon::Impl::Initialize()
{
    /// Fill the cpu model
    CpuInstant cpuInstant;
    cpuInstant[Cpu::User] = 0.0;
    cpuInstant[Cpu::Kernel] = 0.0;
    cpuInstant[Cpu::Idle] = 0.0;
    cpuInstant[Cpu::IoWait] = 0.0;
    cpuInstant[Cpu::Swap] = 0.0;
    cpuInstant[Cpu::Nice] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        CpuUsageCache.push_back(cpuInstant);


    /// Fill the memory model
    MemoryInstant memoryInstant;
    memoryInstant[Memory::Total] = 0.0;
    memoryInstant[Memory::Free] = 0.0;
    memoryInstant[Memory::Used] = 0.0;
    memoryInstant[Memory::Cache] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        MemoryUsageCache.push_back(memoryInstant);


    /// Fill the swap model
    SwapInstant swapInstant;
    swapInstant[Swap::Total] = 0.0;
    swapInstant[Swap::Used] = 0.0;
    swapInstant[Swap::Free] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        SwapUsageCache.push_back(swapInstant);


    /// Fill the virtual memory model
    VirtualMemoryInstant virtualMemoryInstant;
    virtualMemoryInstant[VirtualMemory::Total] = 0.0;
    virtualMemoryInstant[VirtualMemory::Used] = 0.0;
    virtualMemoryInstant[VirtualMemory::Free] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        VirtualMemoryUsageCache.push_back(virtualMemoryInstant);


    /// Initialize and start the timer
    Timer = new Wt::WTimer();
    Timer->setInterval(1000);
    Timer->timeout().connect(this, &SysMon::Impl::RefreshResourceUsage);
}

