/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/


package VjConfig;

import java.util.StringTokenizer;
import java.util.*;
import VjConfig.ValType;
import VjConfig.DescEnum;
import java.io.*;

/** Individual PropertyDesc of a ChunkDesc.
 *  The PropertyDesc represents a single property, with its own name and type
 *  and with one or more values.  The PropertyDesc can include help text,
 *  individual labels for each value of the Property, and an enumeration of
 *  possible values.
 *
 *  Note that PropertyDesc doesn't have any inherent synchronization; anyone
 *  modifying a PropertyDesc needs to guarantee that no one else is looking
 *  at it at the same time.
 *
 *  @author Christopher Just
 *  @version $Revision$
 */
public class PropertyDesc implements Cloneable {

    private String name;
    private String token;
    private String help;
    private int num;
    private ValType valtype;
    private int enumval; // for assigning numeric defaults to enum entries

    // items for assisting in GUI displays of chunks.

    /** User level - 0 for beginner, 1 for expert.  default = 0. */
    private int user_level;

    /** Contains a fixed set of possible values with string labels. */
    private ArrayList enums;

    /** Assigns an individual label to each value of a property. */
    private ArrayList valuelabels;


    public PropertyDesc () {
	/* creates an "empty" PropertyDesc */
	name = "";
	token = "";
	help = "";
	enums = new ArrayList();
	valuelabels = new ArrayList();
	num = 1;
	valtype = ValType.INVALID;
        enumval = 0;
        user_level = 0;
    }



    public Object clone () throws CloneNotSupportedException {
        PropertyDesc p = (PropertyDesc)super.clone();
        p.valuelabels = (ArrayList)valuelabels.clone(); // safe; it's a list of strings
        p.enums = new ArrayList();
        int i, n = enums.size();
        for (i = 0; i < n; i++) {
            p.enums.add (((DescEnum)enums.get(i)).clone());
        }
        return p;
    }


    public void setName (String _name) {
        name = _name;
    }

    public String getName () {
        return name;
    }

    public void setToken (String _token) {
        token = _token;
    }

    public String getToken () {
        return token;
    }

    public void setHelp (String _help) {
        help = _help;
    }

    public String getHelp () {
        return help;
    }

    public void setValType (ValType t) {
        valtype = t;
    }

    public ValType getValType () {
        return valtype;
    }

    public void setHasVariableNumberOfValues (boolean b) {
        if (b)
            num = -1;
        else
            num = 1;
    }

    public boolean getHasVariableNumberOfValues () {
        return (num == -1);
    }

    public void setNumValues (int n) {
        num = n;
    }

    public int getNumValues () {
        return num;
    }

    public void setUserLevel (int level) {
        user_level = level;
    }

    public int getUserLevel () {
        return user_level;
    }

    public void appendValueLabel (String label) {
        valuelabels.add (label);
    }

    public int getValueLabelsSize () {
        return valuelabels.size();
    }

    public String getValueLabel (int i) {
        if (i < valuelabels.size())
            return (String)valuelabels.get(i);
        else
            return "";
    }

    public String[] getValueLabels () {
        String[] e = new String[valuelabels.size()];
        return (String[])valuelabels.toArray(e);
    }

    /* c had better be a collection of String, or things will get
     * real ugly real fast.
     */
    public void setValueLabels (Collection c) {
        valuelabels.clear();
        valuelabels.addAll (c);
    }

    public void appendEnumeration (String label, String value) {
        DescEnum d;
        VarValue v;
        if (value == "") {
            /* no explicit value */
            if (valtype == ValType.STRING ||
                valtype == ValType.CHUNK ||
                valtype == ValType.EMBEDDEDCHUNK)
                v = new VarValue(label);
            else
                v = new VarValue(enumval++);
        }
        else {
            v = new VarValue(valtype);
            v.set (value);
            /* explicit value */
//              VarValue val = new VarValue(valtype);
//              val.set (value);
//              d = new DescEnum (label, val);
        }
        enums.add (new DescEnum (label, v));
    }


    /* c had better be a collection of DescEnums, or things will get
     * real ugly real fast.
     */
    public void setEnumerations (Collection c) {
        enums.clear();
        enums.addAll (c);
    }

    public DescEnum[] getEnumerations () {
        DescEnum[] e = new DescEnum[enums.size()];
        return (DescEnum[])enums.toArray(e);
    }

    public int getEnumerationsSize() {
        return enums.size();
    }

    public DescEnum getEnumAtIndex (int ind) {
	return (DescEnum)enums.get(ind);
    }



    public boolean equals(PropertyDesc d) {
	DescEnum e1, e2;
	if (d == null)
	  return false;
	if (!name.equalsIgnoreCase(d.name))
	    return false;
	if (!token.equalsIgnoreCase(d.token))
	    return false;
	if (!help.equalsIgnoreCase(d.help))
	    return false;
	if (num != d.num)
	    return false;
	if (valtype != d.valtype)
	    return false;

	/* KLUDGE: This next part returns false if both
	 * PropertyDescs have the same descenums in a
	 * different order.  I'm not sure if the enums
	 * code enforces any kind of order or not, or
	 * if this is reasonable behavior. This should
	 * be examined more carefully.
	 */
	for (int i = 0; i <enums.size(); i++) {
	    try {
		e1 = (DescEnum)enums.get(i);
		e2 = (DescEnum)d.enums.get(i);
		if (!e1.equals(e2))
		    return false;
	    }
	    catch (ArrayIndexOutOfBoundsException e) {
		return false;
	    }
	}
	return true;
    }



    public String toString() {
        return xmlRep ("");
    }



    public String xmlRep (String pad) {
        int i, n;
        DescEnum e;
        String newpad = pad + pad;
        // string buffer is a lot more painful code-wise but noticeably 
        // faster.
        StringBuffer retval = new StringBuffer (256);
        retval.append(pad);
        retval.append("<PropertyDesc token=\"");
        retval.append(XMLConfigIOHandler.escapeString(token));
        retval.append("\" name=\"");
        retval.append(XMLConfigIOHandler.escapeString(name));
        if (user_level != 0) {
            retval.append("\" userlevel=\"expert");
        }
        retval.append("\" type=\"");
        retval.append(valtype.toString());
        retval.append("\" num=\"");
        if (num == -1)
            retval.append ("variable\">\n");
        else {
            retval.append (num);
            retval.append ("\">\n");
        }
        if (!help.equals ("")) {
            retval.append(newpad);
            retval.append("<help>");
            retval.append(XMLConfigIOHandler.escapeString(help));
            retval.append("</help>\n");
        }
        n = valuelabels.size();
        if (n > 0) {
            for (i = 0; i < n; i++) {
                retval.append(newpad);
                retval.append("<label name=\"");
                retval.append(XMLConfigIOHandler.escapeString((String)valuelabels.get(i)));
                retval.append("\"/>\n");
            }
        }
        n = enums.size();
        if (n > 0) {
            for (i = 0; i < n; i++) {
                e = (DescEnum)enums.get(i);
                retval.append(newpad);
                retval.append("<enumeration name=\"");
                retval.append(XMLConfigIOHandler.escapeString(e.str));
                if (valtype == ValType.STRING ||
                    valtype == ValType.CHUNK ||
                    valtype == ValType.EMBEDDEDCHUNK)
                    retval.append("\"/>\n");
                else {
                    retval.append("\" value=\"");
                    retval.append(XMLConfigIOHandler.escapeString(e.val.toString()));
                    retval.append("\"/>\n");
                }
            }
        }
        retval.append(pad);
        retval.append("</PropertyDesc>\n");
        return retval.toString();
    }



    /** Maps a string to an enumeration value.
     *  @return The VarValue which is mapped by val, or null if no such
     *          mapping exists.
     */
    public VarValue getEnumValue(String val) {
	/* returns the value associated with this enum el */
	DescEnum t;
 	VarValue v;
	
	for (int i = 0; i < enums.size(); i++) {
	    t = (DescEnum)enums.get(i);
	    if (t.str.equalsIgnoreCase(val)) {
		v = new VarValue(t.val);
		return v;
	    }
	}
        return null;
    }



    public String getEnumString(VarValue val) {
	/* does the reverse mapping of getEnumVal - maps a value 
	 * back to the name of the enum entry 
	 */
	DescEnum t;

	for (int i = 0; i < enums.size(); i++) {
	    t = (DescEnum)enums.get(i);
	    if (t.val.equals(val)) {
		return t.str;
	    }
	}
	return val.toString();
    }


}


