#!/usr/bin/env python

from __future__ import print_function
import sys
import os, os.path
import re
import string
import argparse
import ConfigParser

WORKAROUND_XTENSA_LITERAL = 1


class GnuMapFile:

    def __init__(self, f):
        self.f = f
        self.l = None
        self.section_order = []
        self.sections = {}

    def get(self):
        if self.l is not None:
            l = self.l
            self.l = None
            return l
        return next(self.f)

    def unget(self, l):
        assert self.l is None
        self.l = l

    def peek(self):
        if self.l is None:
            self.l = next(self.f)
        return self.l

    def skip_till_memmap(self):
        for l in self.f:
            if l == "Linker script and memory map\n":
                break

    def skip_while_lead_space(self):
        while 1:
            if self.l is not None and self.l[0] not in [" ", "\n"]:
                break
            self.l = next(self.f)

    def parse_section_line(self):
        dram_addr = 0
        l = self.get().rstrip()
        print(l)
        if l[1] == " " and l[38] != " ":
            fields = l.split(None, 2)
            fields = [""] + fields
        else:
            fields = l.split()
            if " = " in l or "linker stubs" in l or "PROVIDE " in l:
                return [fields[0], 0, 0, 0]
            if len(fields) == 1:
                # For long section names, continuation on 2nd line
                fields2 = self.get().split()
                if len(fields2) < 2:
                    return [fields[0], 0, 0, 0]
                else:
                    fields += fields2
            if l.find("load address") != -1:
                dram_addr = int(fields[5], 0)

        return [fields[0], int(fields[1], 0), int(fields[2], 0), dram_addr] + fields[3:]

    def parse_section(self):
        sec_name, addr, sz, dram_addr = self.parse_section_line()[0:4]
        if sz == 0:
            # logging.debug("Empty section: %s", sec_name)
            self.skip_while_lead_space()
            return

        #print("[parse_section]:", sec_name, addr, sz, dram_addr)
        self.section_order.append((sec_name, addr, sz, dram_addr))
        self.sections[sec_name] = {"addr": addr, "size": sz, "dram": dram_addr}

        last_obj = None
        while 1:
            l = self.get()
            if l.startswith("LOAD ") or l.startswith("START ") or l.startswith("END ") or l.startswith("OUTPUT "):
                self.unget(l)
                break

            if l == "\n":
                break

            if len(l) > 16 and l[16] == " ":
                # assert "(size before relaxing)" in l
                continue

            if l[1] == " " and l[38] == " ":
                addr, sym = l.split(None, 1)
                # print((addr, sym))
                if " = " in sym or "PROVIDE " in sym or "ASSERT " in sym or "ABSOLUTE" in sym:
                    # special symbol, like ABSOLUTE, ALIGN, etc.
                    #print("special symbol", sym)
                    continue
                self.sections[sec_name]["objects"][-1][-1].append((int(addr, 0), sym.rstrip()))
                continue

            if l.startswith(" *fill* "):
                self.unget(l)
                fields = self.parse_section_line()
                if fields[2] == 0:
                    continue
                if last_obj is None:
                    name = sec_name + ".initial_align"
                else:
                    name = last_obj + ".fill"
                self.sections[sec_name].setdefault("objects", []).append(fields + [name, []])
                continue

            if l.find(".a:") != -1:
                continue

            if l.find(".o(") != -1:
                continue

            if l.startswith(" *"):
                # section group "comment"
                continue
            # print("*", l)

            self.unget(l)
            fields = self.parse_section_line()
            if fields[2] == 0:
                if WORKAROUND_XTENSA_LITERAL:
                    # Empty *.literal sub-section followed by
                    # non-empty fill is actually non-empty *.literal
                    if fields[0].endswith(".literal"):
                        if self.peek().startswith(" *fill* "):
                            fields2 = self.parse_section_line()
                            if fields2[2] != 0:
                                # assert 0, (fields, fields2)
                                fields[2] = fields2[2]
                                fields[-1] += ".literal"
                                self.sections[sec_name].setdefault("objects", []).append(fields + [[]])
                continue

            # If an absolute library name (libc, etc.), use the last component
            # if fields[-1][0] == "/":
            #     fields[-1] = fields[-1].rsplit("/", 1)[1]

            self.sections[sec_name].setdefault("objects", []).append(fields + [[]])
            last_obj = fields[-1]
            # assert 0, fields

    def parse_sections(self):
        while 1:
            l = self.get().rstrip()
            if l.startswith("LOAD ") and len(l.split()) == 2:
                continue
            elif l.startswith("START ") and len(l.split()) == 2:
                continue
            elif l.startswith("END ") and len(l.split()) == 2:
                continue
            elif l == "":
                continue
            elif l.startswith("OUTPUT"):
                break
            else:
                self.unget(l)
                self.parse_section()

    # Validate that all sub-sections are adjacent (and thus don't overlap),
    # and fill in parent section completely. Note that read-only string
    # section may be subject of string merging optimization by linker,
    # and fail adjacency check. To "fix" this, "M" flag on ELF section
    # should be unset (e.g. with objcopy --set-section-flags)
    def validate(self):
        for k, sec_addr, sec_sz, dram_addr in self.section_order:
            next_addr = sec_addr
            for sec, addr, sz, dram_addr, obj, symbols in self.sections[k]["objects"]:
                if addr != next_addr:
                    next_addr = addr
                next_addr += sz
            assert next_addr <= sec_addr + sec_sz, "0x%x vs 0x%x" % (next_addr, sec_addr + sec_sz)

def parse_path(path):
    if path.find('mtkeda') != -1:
        return path
    else:
        return '/'.join(path.split('_intermediates/')[-1].split('/')[3:])

def query_feature(features, path):
    feature = 'Platform'; subFeature = ''

    if path.find('heap') != -1:
        feature = 'Heap'
    elif path.find('HEAP') != -1:
        feature = 'Heap'
    elif path.find('RTOS') != -1:
        feature = 'RTOS'
    elif path.find('mtkeda') != -1:
        feature = 'C-lib'
    else:
        for k in features.iterkeys():
            if path.find(k) != -1:
                feature = (features[k])[0]
                subFeature = (features[k])[1]
                break
    return (feature, subFeature)


def main():
    p = argparse.ArgumentParser(prog='memoryReport.py', add_help=True,
                                description="Report memory map by features")
    p.add_argument('-d', action='store_true', default=False, dest='dump', help='dump all output')
    p.add_argument('-e', action='store_true', default=False, dest='excel', help='dump all output to excel')
    p.add_argument('projType', help='project type', metavar='Project_Type')
    p.add_argument('iniFile', help='feature list', metavar='Feature_List')
    p.add_argument('mapFile', help='memory map', metavar='Memory_Map')
    # p.add_argument('prefix', help='prefix', metavar='Path_Prefix')
    args = vars (p.parse_args())
    dump = args['dump']
    excel = args['excel']
    projType = args['projType'].upper()
    iniFile = args['iniFile']
    mapFile = args['mapFile']
    # prefix = args['prefix']
    platform = os.environ.get('PLATFORM').lower()
    settings = []
    features = {}
    sectionS = set()
    featureS = set(['Platform', 'Heap', 'RTOS', 'C-lib'])
    subFeatureS = set()
    sensys = set()
    records = []
    result1 = {}
    result2 = {}
    result_dram = {}       #dram size sum
    dram_section_addr = {} #section to address
    dram_address_size = {} #address to size
    dram_address_maxs  = {} #address of max section
    criteria = {}
    chart = None
    bar = None
    wb = None
    ws1 = None
    ws2 = None
    index1x = 0
    index1y = 0
    index2 = 0
    chartMin = 0
    chartMax = 0
    chartL = 0
    chartD = 0
    barMin = 0
    barMax = 0
    barL = 0
    barD = 0
    platformSection = ''
    ret = 0

    if projType == 'SCP':
        settings = ['TinySys-COMMON', 'TinySys-SCP']

    f = open(iniFile)
    config = ConfigParser.ConfigParser()
    config.optionxform = lambda option: option
    config.readfp(f)

    for i in settings:
        for j in config.items(i):
            features[j[0]] = j[1].strip().split(':')
            featureS.add(features[j[0]][0])
            subFeatureS.add(features[j[0]][1])

    platformSection = projType
    if config.has_section(platformSection):
        for i in config.items(platformSection):
            criteria[i[0]] = i[1]

    for i in config.items('TinySys-SenSys'):
        sensys.add(i[0])

    f.close()
    featureS = filter(None, featureS)
    subFeatureS = filter(None, subFeatureS)

    f = open(mapFile)
    m = GnuMapFile(f)
    m.skip_till_memmap()
    m.skip_while_lead_space()
    m.parse_sections()
    m.validate()

    for k, addr, sz, dram_addr in m.section_order:
        #print("[secorder]:", k, addr, sz)
        section = k; size = sz
        if k.find('.debug_') != -1:
            continue
        sectionS.add(k)

        if (dram_addr > 0):
            dram_section_addr[k] = addr
            #print("dram_section_addr[%s]=%x" %(k, addr))
            if (addr not in dram_address_size) or (dram_address_size[addr] < sz):
                dram_address_size[addr] = sz
                dram_address_maxs[addr] = k
            #print("dram_address_size[%x]=%d" % (addr, dram_address_size[addr]))
        else:
            dram_section_addr[k] = 0
            dram_address_size[addr] = 0

        for sec, addr, sz, dram_addr, obj, symbols in m.sections[k]['objects']:
            #print("[section]: ", sec, addr, sz)
            symbol = '.'.join((sec.split('.'))[2:])
            address = addr; size = sz
            # path = obj.replace(prefix, '')
            path = parse_path(obj)
            [feature, subFeature] = query_feature(features, path)
            if symbols and len(symbols) > 1:
                for i in range(len(symbols) -1):
                    address = symbols[i][0]; symbol = symbols[i][1]
                    # print("  %08x %s" % (address, symbol))
                    records.append([symbol, feature, subFeature, section, address, int(symbols[i+1][0]) - int(address), path])
                i += 1
                records.append([symbols[i][1], feature, subFeature, section, symbols[i][0], int(addr) + int(sz) - int(symbols[i][0]), path])
            elif len(symbols) == 1:
                i = 0
                records.append([symbols[i][1], feature, subFeature, section, symbols[i][0], size, path])
            else:
                records.append((symbol, feature, subFeature, section, address, size, path))

    f.close()

    for s in sectionS:
        for f in featureS:
            result1[(f, s)] = 0
        for f in subFeatureS:
            result2[(f, s)] = 0

    if excel:
        from openpyxl import Workbook
        from openpyxl.chart import Reference, Series, BarChart

        chartL = 1
        chartD = len(sectionS) + 2
        barL = 1
        barD = len(subFeatureS) + 2
        chart = BarChart()
        chart.type = "col"
        chart.grouping = "stacked"
        chart.overlap = 100
        chart.height = 13
        chart.width = 26
        chart.title = "Feature Overall"
        bar = BarChart()
        bar.type = "col"
        bar.grouping = "stacked"
        bar.overlap = 100
        bar.height = 13
        bar.width = 26
        bar.title = "Sub-feature View"
        wb = Workbook()
        ws1 = wb.create_sheet("Memory Report", 0)
        ws2 = wb.create_sheet("Symbols", 1)
        ws2.cell(row=1, column=1, value='symbol')
        ws2.cell(row=1, column=2, value='feature')
        ws2.cell(row=1, column=3, value='sub_feature')
        ws2.cell(row=1, column=4, value='section')
        ws2.cell(row=1, column=5, value='address')
        ws2.cell(row=1, column=6, value='size')
        ws2.cell(row=1, column=7, value='path')
        index2 = 1

    for symbol, feature, subFeature, section, address, size, path in records:
        if dump: print("%-60s\t%-9s\t%-9s\t%-13s\t0x%08x\t%s\t%s" % (symbol, feature, subFeature, section, address, size, path))
        if excel:
            index2 = index2 + 1
            ws2.cell(row=index2, column=1, value=symbol)
            ws2.cell(row=index2, column=2, value=feature)
            ws2.cell(row=index2, column=3, value=subFeature)
            ws2.cell(row=index2, column=4, value=section)
            ws2.cell(row=index2, column=5, value=address)
            ws2.cell(row=index2, column=6, value=size)
            ws2.cell(row=index2, column=7, value=path)

        if feature and section:
            result1[(feature, section)] += size
        if subFeature and section:
            result2[(subFeature, section)] += size
    #Feature report starts
    if dump: print(); print(); print()
    if dump: print("%20s\t" % '', end='')
    if excel: index1x = 1; index1y = 1; chartMin = index1y + 1
    #Sections in SRAM
    for s in sorted(sectionS):
        if dump: print("%20s\t" % s, end='')
        if excel:
            if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                index1x += 1
                ws1.cell(row=index1y, column=index1x, value=s)
    if dump: print("%20s" % "Sum")
    if excel:
        index1x += 1
        ws1.cell(row=index1y, column=index1x, value="SRAM")
    #Sections in DRAM
        for s in sorted(sectionS):
            if (s == '.dram_region') or (dram_section_addr[s] != 0):
                index1x += 1
                ws1.cell(row=index1y, column=index1x, value=s)
        index1x += 1
        ws1.cell(row=index1y, column=index1x, value="DRAM")
    #Features in SRAM
    for f in sorted(featureS):
        if dump: print("%20s\t" % f, end='')
        if excel:
            index1y += 1
            index1x = 1
            ws1.cell(row=index1y, column=index1x, value=f)
        result1[(f, 'sum')] = 0
        for s in sorted(sectionS):
            v = result1.get((f, s), 0)
            if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                result1[(f, 'sum')] += v
            if dump: print("%20d\t" % v, end='')
            if excel:
                if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                    index1x += 1
                    ws1.cell(row=index1y, column=index1x, value=v)
        if dump: print("%20d" % result1[(f, 'sum')])
        if excel:
            index1x += 1
            ws1.cell(row=index1y, column=index1x, value=result1[(f, 'sum')])
    #Features in DRAM
    index1y = 1
    tmpx = index1x
    for f in sorted(featureS):
        result_dram[f] = 0
        index1x = tmpx
        index1y += 1
        for s in sorted(sectionS):
           if (s == '.dram_region') or (dram_section_addr[s] != 0):
               index1x += 1
               v = result1.get((f, s), 0)
               result_dram[f] += v
               if excel:
                   ws1.cell(row=index1y, column=index1x, value=v)
        index1x += 1
        if excel:
            ws1.cell(row=index1y, column=index1x, value=result_dram[f])
    if excel: chartMax = index1y

    #Overlay address
    index1x = 1
    index1y += 2
    if excel:
        ws1.cell(row=index1y, column=index1x, value="Overlay Offset")
        for s in sorted(sectionS):
            if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                index1x += 1
                if (dram_section_addr[s] > 0):
                    ws1.cell(row=index1y, column=index1x, value=hex(dram_section_addr[s]))
        index1x += 1
        for s in sorted(sectionS):
            if (s == '.dram_region') or (dram_section_addr[s] != 0):
                index1x += 1
                if (dram_section_addr[s] > 0):
                    ws1.cell(row=index1y, column=index1x, value=hex(dram_section_addr[s]))

    #Sub-feature report starts
    if dump: print(); print(); print()
    if dump: print("%20s\t" % '', end='')
    if excel: index1x = 1; index1y += 2; barMin = index1y + 1
    for s in sorted(sectionS):
        if dump: print("%20s\t" % s, end='')
        if excel:
            #Sections in SRAM
            if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                index1x += 1
                ws1.cell(row=index1y, column=index1x, value=s)
    if dump: print("%20s" % "Sum")
    if excel:
        index1x += 1
        ws1.cell(row=index1y, column=index1x, value="SRAM")
        #Sections in DRAM
        for s in sorted(sectionS):
            if (s == '.dram_region') or (dram_section_addr[s] != 0):
                index1x += 1
                ws1.cell(row=index1y, column=index1x, value=s)
        index1x += 1
        ws1.cell(row=index1y, column=index1x, value="DRAM")
    #Sub-features in SRAM
    tmpy = index1y
    for f in sorted(subFeatureS):
        if dump: print("%20s\t" % f, end='')
        if excel:
            index1y += 1
            index1x = 1
            ws1.cell(row=index1y, column=index1x, value=f)
        result2[(f, 'sum')] = 0
        result_dram[f] = 0
        for s in sorted(sectionS):
            v = result2.get((f, s), 0)
            if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                result2[(f, 'sum')] += v

            if dump: print("%20d\t" % v, end='')
            if excel:
                if (s != '.dram_region') and ((dram_section_addr[s] == 0) or (dram_address_maxs[dram_section_addr[s]] == s)):
                    index1x += 1
                    ws1.cell(row=index1y, column=index1x, value=v)

        if dump: print("%20d" % result2[(f, 'sum')])
        if excel:
            index1x += 1
            ws1.cell(row=index1y, column=index1x, value=result2[(f, 'sum')])

    #Sub-features in DRAM
    index1y = tmpy
    tmpx = index1x
    for f in sorted(subFeatureS):
        result_dram[f] = 0
        index1x = tmpx
        index1y += 1
        for s in sorted(sectionS):
            if (s == '.dram_region') or (dram_section_addr[s] != 0):
                index1x += 1
                v = result2.get((f, s), 0)
                result_dram[f] += v
                if excel:
                    ws1.cell(row=index1y, column=index1x, value=v)
        index1x += 1
        if excel:
            ws1.cell(row=index1y, column=index1x, value=result_dram[f])

    if excel: barMax = index1y

    if excel:
        labels = Reference(ws1, min_col=chartL, min_row=chartMin, max_row=chartMax)
        data = Reference(ws1, min_col=2, min_row=1, max_col=chartD-1, max_row=chartMax)
        chart.add_data(data=data, titles_from_data=True)
        chart.set_categories(labels)
        ws1.add_chart(chart, "A%d" % (barMax+4))

        labels = Reference(ws1, min_col=barL, min_row=barMin, max_row=barMax)
        data = Reference(ws1, min_col=2, min_row=barMin-1, max_col=barD-1, max_row=barMax)
        bar.add_data(data=data, titles_from_data=True)
        bar.set_categories(labels)
        ws1.add_chart(bar, "A%d" % (barMax+4+24+4))

        wb.save('memory_report.xlsx')

    if config.has_section(platformSection):
        if dump: print(); print()
        print()
        for f in sorted(featureS):
            if f == 'Peripheral':
                continue
            c = int(criteria.get(f, 0))
            m = int(result1.get((f, 'sum'), 0))
            if m > c:
                print("%s: %s(%d>%d) is out of memory limitation" % (projType, f, m, c))
                ret = 13

        if sensys != set([]):
            c = int(criteria.get('SenSys', 0))
            m = 0
            for f in sensys:
                m += int(result1.get((f, 'sum'), 0))
            if m > c:
                print("%s: %s(%d>%d) is out of memory limitation" % (projType, 'SenSys', m, c))
                ret = 13
        for f in sorted(subFeatureS):
            c = int(criteria.get(f, 0))
            m = int(result2.get((f, 'sum'), 0))
            if m > c:
                print("%s: %s(%d>%d) is out of memory limitation" % (projType, f, m, c))
                ret = 13

        if ret == 0: print("%s: pass memory limitation check" %(projType)); print()
    return ret

if '__main__'==__name__:
        ret = main()
        sys.exit(ret)
