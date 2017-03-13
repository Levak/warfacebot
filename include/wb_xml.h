/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015-2017 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WB_XML_H
# define WB_XML_H

/*
 * Allocate a copy of the given string and replace any instance
 * of   & < > ' "    by     &amp; &lt; &gt; &apos; &quot;
 */
char *xml_serialize(const char *str);

/*
 * Inplace (realloc) replace any instance
 * of   & < > ' "    by     &amp; &lt; &gt; &apos; &quot;
 */
char *xml_serialize_inplace(char **str);

/*
 * Allocate a copy of the given string and replace any instance
 * of   &amp; &lt; &gt; &apos; &quot;    by     & < > ' "
 */
char *xml_deserialize(const char *str);

/*
 * Inplace (realloc) replace any instance
 * of   &amp; &lt; &gt; &apos; &quot;    by     & < > ' "
 */
char *xml_deserialize_inplace(char **str);

#endif /* !WB_XML_H */
