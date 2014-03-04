
#include "gpx_loader.h"
#include "xml_helper.h"


static struct GPSGroup* gpx_node(mxml_node_t* node) {
	struct GPSGroup* g_header = NULL;
	struct GPSGroup* g_offset = NULL;

	mxml_node_t* gpx = mxmlFindElement(node, node, "gpx", NULL, NULL, MXML_DESCEND);
	if ( gpx ) {

		mxml_node_t* trkseg = mxmlFindElement(gpx, node, "trkseg", NULL, NULL, MXML_DESCEND);
		while (trkseg) {

			struct GPSGroup* g = (struct GPSGroup*)calloc(1, sizeof(struct GPSGroup));
			struct GPSPoint* p_offset = NULL;
			struct GPSPoint* st = NULL;
			struct GPSPoint* et = NULL;

			mxml_node_t* trkpt = mxmlFindElement(trkseg, gpx, "trkpt", NULL, NULL, MXML_DESCEND);
			while ( trkpt ) {

				mxml_node_t* ele = mxmlFindElement(trkpt, trkpt, "ele", NULL, NULL, MXML_DESCEND_FIRST);
				mxml_node_t* time = mxmlFindElement(trkpt, trkpt, "time", NULL, NULL, MXML_DESCEND_FIRST);

				if ( ele && time ) {
					struct GPSPoint* p = (struct GPSPoint*)calloc(1, sizeof(struct GPSPoint));

					p->x = strtod(mxmlElementGetAttr(trkpt, "lat"), NULL);
					p->y = strtod(mxmlElementGetAttr(trkpt, "lon"), NULL);
					p->z = strtod(mxmlGetText(ele, NULL), NULL);

					gpxtime_to_exiftime(p->time, mxmlGetText(time, NULL));

					if ( !st || tmcmp(p->time, st->time) < 0 ) {
						st = p;
					}
					
					if ( !et || tmcmp(p->time, et->time) > 0 ) {
						et = p;
					}

					if ( !g->point ) {
						g->point = p;
					}

					if ( p_offset ) {
						p_offset->next = p;
					}
					p_offset = p;

					++g->number;
				}

				trkpt = mxmlFindElement(trkpt, trkseg, "trkpt", NULL, NULL, MXML_DESCEND);
			}

			if ( g->number > 0 ) {
				if ( st ) {
					strncpy(g->start_time, st->time, sizeof(g->start_time));
				}

				if ( et ) {
					strncpy(g->end_time, et->time, sizeof(g->start_time));
				}

				if ( !g_header ) {
					g_header = g;
				}
				if ( g_offset ) {
					g_offset->next = g;
				}
				g_offset = g;
			} else {
				free(g);
			}

			trkseg = mxmlFindElement(trkseg, gpx, "trkseg", NULL, NULL, MXML_DESCEND);
		}
	}
	return g_header;
}


struct GPSGroup* gpx_loader(const char* filename) {
	mxml_node_t* root = xml_load(filename);
	if ( root ) {
		struct GPSGroup* gpx = gpx_node(root);
		xml_close(root);
		return gpx;
	}
	return NULL;
}


