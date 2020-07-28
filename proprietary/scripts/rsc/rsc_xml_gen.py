#!/usr/bin/python
# -*- coding: UTF-8 -*-

from xml.dom import minidom
import os
import sys,re

class RscXml(object):
    RSC_XML_VERSION = '1'
    #——————————————XML define————————————-
    NODE_ROOT = 'runtime_switchable_config'
    NODE_ROOT_ATTR_VER = 'version'
    NODE_ROOT_ATTR_TYPE = 'type'
    NODE_LIST = 'proj_info'
    NODE_ITEM = 'proj_item'
    NODE_ITEM_ATTR_IDX = 'index'
    NODE_ITEM_NAME = 'name'
    NODE_ITEM_OPTR = 'operator'
    #———————————————————————————————————————

    def convertProjectToRscName(self, project):
        result = re.search(r'\[.*\]$', project)
        if result:
            return result.group()
        else:
            return "[default]"

    def createEmptyRscXmlObj(self, xml_type):
        rsc_xml = minidom.Document()
        root_note = rsc_xml.createElement(self.NODE_ROOT)
        root_note.setAttribute(self.NODE_ROOT_ATTR_VER, self.RSC_XML_VERSION)
        root_note.setAttribute(self.NODE_ROOT_ATTR_TYPE, xml_type)
        rsc_xml.appendChild(root_note)
        root_note.appendChild(rsc_xml.createElement(self.NODE_LIST))
        return rsc_xml

    def getRscItem(self, rsc_xml, name):
        for proj_item in rsc_xml.documentElement.getElementsByTagName("proj_item"):
            try:
                cur_name = proj_item.getElementsByTagName('name')[0].childNodes[0].data
                #print cur_name
                if cur_name == name:
                    print "found it {0}".format(cur_name)
                    return proj_item
            except Exception as err:
                print('Error：{0}'.format(err))
        print "Not found it"
        return None

    def setAttrType(self, rsc_xml, xml_type):
        rsc_xml.documentElement.setAttribute(self.NODE_ROOT_ATTR_TYPE, xml_type)

    def setAttrVersion(self, rsc_xml, xml_version):
        rsc_xml.documentElement.setAttribute(self.NODE_ROOT_ATTR_VER, xml_version)

    def addRscItem(self, rsc_xml, index, name):
        #get rsc list obj
        rsc_list = rsc_xml.documentElement.getElementsByTagName(self.NODE_LIST)[0]
        #add a item
        item_node = rsc_xml.createElement(self.NODE_ITEM)
        item_node.setAttribute(self.NODE_ITEM_ATTR_IDX, index)
        rsc_list.appendChild(item_node)
        #add name
        name_node = rsc_xml.createElement(self.NODE_ITEM_NAME)
        name_node.appendChild(rsc_xml.createTextNode(name))
        item_node.appendChild(name_node)
        return item_node

    def addRscItemElement(self, dest_xml, item_element):
        rsc_item = dest_xml.documentElement.getElementsByTagName(self.NODE_LIST)[0]
        rsc_item.appendChild(item_element)
        return item_element

    def copyElementsToRoot(self, dest_xml, source_xml, ele_name):
        source_root = dest_xml.documentElement
        source_root.appendChild(source_xml.documentElement.getElementsByTagName(ele_name)[0])

    def addDtsToRscItem(self, dest_xml, dest_element, kernel_element):
        if kernel_element:
            dts_index = kernel_element.getAttribute(self.NODE_ITEM_ATTR_IDX)
        else:
            dts_index = '0'
        dest_element.setAttribute(self.NODE_ITEM_ATTR_IDX, dts_index)

    def addOptrToRscItem(self, dest_xml, dest_element, source_element):
        operator = source_element.getElementsByTagName(self.NODE_ITEM_OPTR)
        if operator.length != 0:
            dest_element.appendChild(operator[0])
        else:
            optr_node = dest_xml.createElement(self.NODE_ITEM_OPTR)
            optr_node.appendChild(dest_xml.createTextNode('NONE'))
            dest_element.appendChild(optr_node)

    def writeRscXml(self, rsc_xml, filename):
        try:
            with open(filename,'w') as fh:
                rsc_xml.writexml(fh,indent='',addindent='\t',newl='\n',encoding='UTF-8')
        except Exception as err:
            print('Error：{0}'.format(err))

    def readRscXml(self, filename):
        print '------------' + filename + '------------'
        # somefiles include /t, remove it.
        xml = []
        with open(filename, 'r') as f:
            for line in f:
                str = line.strip()
                xml.append(str)
        rsc_xml = minidom.parseString("".join(xml))
        return rsc_xml


def genKernelXml(projects, out_file):
    rsc_list = projects.split(' ')
    rsc = RscXml()
    rsc_xml = rsc.createEmptyRscXmlObj('kernel')
    for i in range(len(rsc_list)):
        rsc.addRscItem(rsc_xml, str(i), rsc.convertProjectToRscName(rsc_list[i]))
    rsc.writeRscXml(rsc_xml, out_file)
    print "Done"


def genFinalXml(system_filename, vendor_filename, kernel_filename, select_filename, out_file):
    rsc = RscXml()
    system_xml = rsc.readRscXml(system_filename)
    vendor_xml = rsc.readRscXml(vendor_filename)
    kernel_xml = rsc.readRscXml(kernel_filename)
    select_xml = rsc.readRscXml(select_filename)
    print "add magic/part_info to select list from Vendor"
    rsc.setAttrType(select_xml, 'final')
    rsc.copyElementsToRoot(select_xml, vendor_xml, "magic")
    rsc.copyElementsToRoot(select_xml, vendor_xml, "part_info")
    print "add other info and des to select list"
    no_select = False
    if select_xml.documentElement.getElementsByTagName("proj_info").length == 0:
        print "No User Select RSC, use Vendor Config"
        no_select = True
        rsc.copyElementsToRoot(select_xml, vendor_xml, "proj_info")

    for proj_item in select_xml.documentElement.getElementsByTagName("proj_item"):
        try:
            name = proj_item.getElementsByTagName('name')[0].childNodes[0].data
            rsc_vendor_item = rsc.getRscItem(vendor_xml, name)
            rsc_system_item = rsc.getRscItem(system_xml, name)
            rsc_kernel_item = rsc.getRscItem(kernel_xml, name)
            if no_select == False:
                if rsc_vendor_item:
                    print "add vendor optr"
                    rsc.addOptrToRscItem(select_xml, proj_item, rsc_vendor_item)
                elif rsc_system_item:
                    print "add system optr"
                    rsc.addOptrToRscItem(select_xml, proj_item, rsc_system_item)
                else:
                    print('Error: not found RSC:{0} in vendor.xml nor system.xml, Demo project config error'.format(name))
            rsc.addDtsToRscItem(select_xml, proj_item, rsc_kernel_item)
        except Exception as err:
            print('Error：{0}'.format(err))

    rsc.writeRscXml(select_xml, out_file)
    print "Done"


if __name__=='__main__':
    print "gen rsc xml, input:" + str(sys.argv)
    if len(sys.argv) == 4 and sys.argv[1] == "-kernel":
        genKernelXml(sys.argv[-2],sys.argv[-1])
    elif len(sys.argv) == 7 and sys.argv[1] == "-final":
        genFinalXml(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6])
    else:
        print 'Command Error, only support 2 commands:\n' + \
        '-kernel: need 2 param example: rsc_xml_gen.py -kernel "project_xxx project_xxx[rsc01] project_xxx[rsc02]" ./rsc_kernel.xml\n' + \
        '-final: need 5 param example: rsc_xml_gen.py -final ./rsc_system.xml ./rsc_vendor.xml ./rsc_kernel.xml ./rsc_user.xml ./rsc.xml'
        os.exit(1)
